// Sailing Heart


#include "Grid/SHGridBase.h"
#include "Block/SHPlayerBlock.h"
#include "Game/SHGameStateBase.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ASHGridBase::ASHGridBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 启用网络复制
	bReplicates = true;
	bAlwaysRelevant = true;

	// 启用移动复制（Grid 移动时客户端同步）
	SetReplicateMovement(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	GridMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GridMesh"));
	GridMesh->SetupAttachment(Root);
	GridMesh->SetCastShadow(false);

	MeshBorderActor = nullptr;
	DecalBorderActor = nullptr;
	CachedGameState = nullptr;
}

void ASHGridBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHGridBase, OccupiedCells);
	DOREPLIFETIME(ASHGridBase, bEnableMovement);
	DOREPLIFETIME(ASHGridBase, MovementSpeed);
	DOREPLIFETIME(ASHGridBase, MovementDirection);
}

void ASHGridBase::GenerateGrid()
{
	GridMesh->ClearAllMeshSections();
	CreateGridLines();
}

void ASHGridBase::UpdateBorderSize_Implementation()
{
	// 默认实现为空 - 在蓝图子类中覆盖此函数来调用 Border 的更新接口
}

void ASHGridBase::InitializeBorder_Implementation()
{
	// 默认实现为空 - 在蓝图子类中覆盖此函数来调用 Border 的更新接口
}

bool ASHGridBase::GetCellIndexFromLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const
{
	// 计算 Grid 左下角的世界坐标（使用实际尺寸）
	const int32 TotalRows = GetTotalRows();
	const int32 TotalColumns = GetTotalColumns();
	const FVector CenterOffset(-TotalColumns * CellSize * 0.5f, -TotalRows * CellSize * 0.5f, 0.0f);
	const FVector GridBottomLeft = GetActorLocation() + GridOrigin + CenterOffset;

	// 将世界坐标转换为相对于 Grid 左下角的局部坐标
	const FVector LocalPos = WorldLocation - GridBottomLeft;

	// 计算单元格索引
	OutColumn = FMath::FloorToInt(LocalPos.X / CellSize);
	OutRow = FMath::FloorToInt(LocalPos.Y / CellSize);

	// 检查是否在 Grid 范围内
	if (OutRow < 0 || OutRow >= TotalRows || OutColumn < 0 || OutColumn >= TotalColumns)
	{
		return false;
	}

	return true;
}

FVector ASHGridBase::GetCellCenterLocation(const int32 Row, const int32 Column) const
{
	// 计算 Grid 左下角的世界坐标（使用实际尺寸）
	const int32 TotalRows = GetTotalRows();
	const int32 TotalColumns = GetTotalColumns();
	const FVector CenterOffset(-TotalColumns * CellSize * 0.5f, -TotalRows * CellSize * 0.5f, 0.0f);
	const FVector GridBottomLeft = GetActorLocation() + GridOrigin + CenterOffset;

	// 计算单元格中心位置
	const float CellCenterX = (Column + 0.5f) * CellSize;
	const float CellCenterY = (Row + 0.5f) * CellSize;

	return GridBottomLeft + FVector(CellCenterX, CellCenterY, 0.0f);
}

bool ASHGridBase::IsCellValid(const int32 Row, const int32 Column) const
{
	// 实际 Grid 尺寸是 (Rows+2) x (Columns+2)
	return Row >= 0 && Row < GetTotalRows() && Column >= 0 && Column < GetTotalColumns();
}

bool ASHGridBase::ContainsWorldLocation(const FVector& WorldLocation) const
{
	int32 Row, Column;
	return GetCellIndexFromLocation(WorldLocation, Row, Column);
}

ASHPlayerBlock* ASHGridBase::GetShipBlockAt(int32 Row, int32 Column) const
{
	const FIntPoint Key(Column, Row);
	const ASHPlayerBlock* const* Found = CellData.Find(Key);
	return Found ? const_cast<ASHPlayerBlock*>(*Found) : nullptr;
}

void ASHGridBase::SetShipBlockAt(const int32 Row, const int32 Column, ASHPlayerBlock* Block)
{
	const FIntPoint Key(Column, Row);
	if (Block)
	{
		CellData.Add(Key, Block);
		// 添加到已占用列表（用于复制给中途加入的客户端）
		OccupiedCells.AddUnique(Key);
		MulticastEnableCellCollision(Row, Column, true);

		// 将方块附加到 Grid（移动 Grid 时方块跟随）
		Block->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		CellData.Remove(Key);
		OccupiedCells.Remove(Key);
		MulticastEnableCellCollision(Row, Column, false);
	}
}

void ASHGridBase::ClearShipBlockAt(const int32 Row, const int32 Column)
{
	const FIntPoint Key(Column, Row);

	// 解除方块附加
	ASHPlayerBlock* Block = GetShipBlockAt(Row, Column);
	if (Block)
	{
		Block->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	CellData.Remove(Key);
	OccupiedCells.Remove(Key);
	MulticastEnableCellCollision(Row, Column, false);
}

bool ASHGridBase::HasShipBlockAt(const int32 Row, const int32 Column) const
{
	const FIntPoint Key(Column, Row);
	return CellData.Contains(Key) && CellData[Key] != nullptr;
}

ASHGridBase* ASHGridBase::FindGridAtLocation(const UObject* WorldContextObject, const FVector& WorldLocation)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 查找所有 GridActor
	TArray<AActor*> FoundGrids;
	UGameplayStatics::GetAllActorsOfClass(World, ASHGridBase::StaticClass(), FoundGrids);

	// 找到包含该位置的 GridActor
	for (AActor* Actor : FoundGrids)
	{
		ASHGridBase* Grid = Cast<ASHGridBase>(Actor);
		if (Grid && Grid->ContainsWorldLocation(WorldLocation))
		{
			return Grid;
		}
	}

	return nullptr;
}

ASHGridBase* ASHGridBase::GetPlayerFacingCellInfo(const UObject* WorldContextObject, APawn* PlayerPawn,
	FVector& OutLocation, int32& OutRow, int32& OutColumn)
{
	if (!WorldContextObject || !PlayerPawn)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 首先找到玩家所在的 Grid
	ASHGridBase* PlayerGrid = FindGridAtLocation(WorldContextObject, PlayerPawn->GetActorLocation());
	if (!PlayerGrid)
	{
		return nullptr;
	}

	// 获取玩家在当前Grid中的单元格索引
	int32 PlayerRow, PlayerColumn;
	if (!PlayerGrid->GetCellIndexFromLocation(PlayerPawn->GetActorLocation(), PlayerRow, PlayerColumn))
	{
		return nullptr;
	}

	// 获取玩家面朝方向
	FVector ForwardDir = PlayerPawn->GetActorForwardVector();

	// 将面朝方向转换为网格方向（4方向）
	int32 DeltaRow = 0;
	int32 DeltaColumn = 0;

	if (FMath::Abs(ForwardDir.X) > FMath::Abs(ForwardDir.Y))
	{
		DeltaColumn = (ForwardDir.X > 0) ? 1 : -1;
	}
	else
	{
		DeltaRow = (ForwardDir.Y > 0) ? 1 : -1;
	}

	// 计算面朝方向的单元格（在当前Grid中）
	const int32 FacingRow = PlayerRow + DeltaRow;
	const int32 FacingColumn = PlayerColumn + DeltaColumn;

	// 首先检查是否在当前Grid范围内
	if (PlayerGrid->IsCellValid(FacingRow, FacingColumn))
	{
		OutRow = FacingRow;
		OutColumn = FacingColumn;
		OutLocation = PlayerGrid->GetCellCenterLocation(FacingRow, FacingColumn);
		return PlayerGrid;
	}

	// 如果不在当前Grid范围内，计算面朝单元格的世界坐标
	// 使用当前Grid的单元格大小来估算位置
	FVector FacingWorldLocation = PlayerGrid->GetCellCenterLocation(PlayerRow, PlayerColumn);
	FacingWorldLocation.X += DeltaColumn * PlayerGrid->CellSize;
	FacingWorldLocation.Y += DeltaRow * PlayerGrid->CellSize;

	// 查找是否有其他Grid包含这个位置
	TArray<AActor*> FoundGrids;
	UGameplayStatics::GetAllActorsOfClass(World, ASHGridBase::StaticClass(), FoundGrids);

	for (AActor* Actor : FoundGrids)
	{
		ASHGridBase* OtherGrid = Cast<ASHGridBase>(Actor);
		if (OtherGrid && OtherGrid != PlayerGrid)
		{
			int32 OtherRow, OtherColumn;
			if (OtherGrid->GetCellIndexFromLocation(FacingWorldLocation, OtherRow, OtherColumn))
			{
				OutRow = OtherRow;
				OutColumn = OtherColumn;
				OutLocation = OtherGrid->GetCellCenterLocation(OtherRow, OtherColumn);
				return OtherGrid;
			}
		}
	}

	// 没有找到任何Grid包含面朝的位置
	return nullptr;
}

bool ASHGridBase::IsCellPlaceable(int32 Row, int32 Column) const
{
	// 可放置区域是 [1, Rows] x [1, Columns]（排除边缘一圈）
	return Row >= 1 && Row <= Rows && Column >= 1 && Column <= Columns;
}

void ASHGridBase::BeginPlay()
{
	Super::BeginPlay();

	// 缓存 GameState 指针
	CachedGameState = GetWorld()->GetGameState<ASHGameStateBase>();

	// 如果是玩家Grid，注册到GameState
	if (HasAuthority() && GridOwnerType == EGridOwnerType::Player && CachedGameState)
	{
		CachedGameState->RegisterPlayerGrid(this);
	}

	SpawnBorders();
	InitializeCellCollisions();
}

void ASHGridBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateGrid();
}

void ASHGridBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Grid 移动逻辑（只在服务器执行）
	if (HasAuthority() && bEnableMovement && MovementDirection != EGridMovementDirection::None)
	{
		// 使用缓存的 GameState 获取全局时间流速
		const float GlobalTimeScale = CachedGameState ? CachedGameState->GetGlobalTimeScale() : 1.0f;

		// 计算移动量：速度 * DeltaTime * 全局时间流速
		const FVector DirectionVector = GetMovementDirectionVector();
		const FVector Movement = DirectionVector * MovementSpeed * DeltaTime * GlobalTimeScale;

		// 移动 Grid
		AddActorWorldOffset(Movement);
	}

	if (!bShowDebugSphere)
	{
		return;
	}

	// 获取玩家 Pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// 获取玩家位置对应的单元格
	int32 Row, Column;
	if (GetCellIndexFromLocation(PlayerPawn->GetActorLocation(), Row, Column))
	{
		// 获取单元格中心位置并画 debug 球（绿色表示玩家所在单元格）
		FVector CellCenter = GetCellCenterLocation(Row, Column);
		DrawDebugSphere(GetWorld(), CellCenter, DebugSphereRadius, 12, DebugSphereColor, false, -1.0f, 0, 2.0f);

		// 获取玩家面朝方向
		FVector ForwardDir = PlayerPawn->GetActorForwardVector();

		// 将面朝方向转换为网格方向（4方向）
		// X轴对应Column，Y轴对应Row
		int32 DeltaRow = 0;
		int32 DeltaColumn = 0;

		// 判断主要方向（选择绝对值较大的轴）
		if (FMath::Abs(ForwardDir.X) > FMath::Abs(ForwardDir.Y))
		{
			// X轴方向为主
			DeltaColumn = (ForwardDir.X > 0) ? 1 : -1;
		}
		else
		{
			// Y轴方向为主
			DeltaRow = (ForwardDir.Y > 0) ? 1 : -1;
		}

		// 计算面朝方向的单元格
		int32 FacingRow = Row + DeltaRow;
		int32 FacingColumn = Column + DeltaColumn;

		// 检查面朝单元格是否在Grid范围内，如果在则画红色圆
		if (IsCellValid(FacingRow, FacingColumn))
		{
			FVector FacingCellCenter = GetCellCenterLocation(FacingRow, FacingColumn);
			// 可放置区域用红色，边缘区域用黄色
			FColor SphereColor = IsCellPlaceable(FacingRow, FacingColumn) ? FColor::Red : FColor::Yellow;
			DrawDebugSphere(GetWorld(), FacingCellCenter, DebugSphereRadius, 12, SphereColor, false, -1.0f, 0, 2.0f);
		}
	}
}

void ASHGridBase::CreateGridLines()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// 使用实际尺寸（包含边缘）
	const int32 TotalRows = GetTotalRows();
	const int32 TotalColumns = GetTotalColumns();

	// 计算网格中心偏移，使GridOrigin位于所有格子的正中间
	const FVector CenterOffset(-TotalColumns * CellSize * 0.5f, -TotalRows * CellSize * 0.5f, 0.0f);

	// 横向线条（行边框）- 从上到下
	for (int32 Row = 0; Row <= TotalRows; ++Row)
	{
		const float Y = Row * CellSize;
		FVector Start = GridOrigin + CenterOffset + FVector(0.0f, Y, 0.0f);
		FVector End = GridOrigin + CenterOffset + FVector(TotalColumns * CellSize, Y, 0.0f);
		AddLine(Start, End, Vertices, Triangles, Normals);
	}

	// 纵向线条（列边框）- 从左到右
	for (int32 Col = 0; Col <= TotalColumns; ++Col)
	{
		const float X = Col * CellSize;
		FVector Start = GridOrigin + CenterOffset + FVector(X, 0.0f, 0.0f);
		FVector End = GridOrigin + CenterOffset + FVector(X, TotalRows * CellSize, 0.0f);
		AddLine(Start, End, Vertices, Triangles, Normals);
	}

	// 为所有顶点填充默认值
	UVs.Init(FVector2D(0, 0), Vertices.Num());
	VertexColors.Init(FColor::White, Vertices.Num());
	Tangents.Init(FProcMeshTangent(0, 1, 0), Vertices.Num());

	// 创建mesh section，不启用碰撞
	GridMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false);

	// 设置材质（如果还没有）
	if (!GridMesh->GetMaterial(0))
	{
		UMaterial* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
		GridMesh->SetMaterial(0, DefaultMaterial);
	}
}

void ASHGridBase::AddLine(const FVector& Start, const FVector& End, TArray<FVector>& Vertices, TArray<int32>& Triangles,
	TArray<FVector>& Normals)
{
	const FVector Direction = (End - Start).GetSafeNormal();
	const FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector) * LineThickness * 0.5f;

	const int32 StartIndex = Vertices.Num();

	// 创建4个顶点构成矩形
	Vertices.Add(Start - Perpendicular);
	Vertices.Add(Start + Perpendicular);
	Vertices.Add(End + Perpendicular);
	Vertices.Add(End - Perpendicular);

	// 添加法线
	for (int32 i = 0; i < 4; ++i)
	{
		Normals.Add(FVector::UpVector);
	}

	// 创建2个三角形（逆时针绕序，从上往下看可见）
	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 2);
	Triangles.Add(StartIndex + 1);

	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 3);
	Triangles.Add(StartIndex + 2);
}

void ASHGridBase::SpawnBorders()
{
	// Border 是纯视觉效果，在每个客户端本地生成
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 生成 Mesh Border
	if (MeshBorderClass && !MeshBorderActor)
	{
		MeshBorderActor = World->SpawnActor<AActor>(MeshBorderClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (MeshBorderActor)
		{
			MeshBorderActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	// 生成 Decal Border
	if (DecalBorderClass && !DecalBorderActor)
	{
		DecalBorderActor = World->SpawnActor<AActor>(DecalBorderClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (DecalBorderActor)
		{
			DecalBorderActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	// 本地初始化 Border
	if (MeshBorderActor && DecalBorderActor)
	{
		InitializeBorder();
		UpdateBorderSize();
	}
}

void ASHGridBase::UpdateGridBorders()
{
	if (MeshBorderActor && DecalBorderActor)
	{
		UpdateBorderSize();
	}
}

void ASHGridBase::InitializeCellCollisions()
{
	// 使用实际尺寸
	const int32 TotalRows = GetTotalRows();
	const int32 TotalColumns = GetTotalColumns();

	// 计算 Grid 左下角的世界坐标
	const FVector CenterOffset(-TotalColumns * CellSize * 0.5f, -TotalRows * CellSize * 0.5f, 0.0f);

	for (int32 Row = 0; Row < TotalRows; ++Row)
	{
		for (int32 Col = 0; Col < TotalColumns; ++Col)
		{
			FIntPoint Key(Col, Row);

			// 创建碰撞盒组件
			UBoxComponent* BoxComp = NewObject<UBoxComponent>(this);
			BoxComp->SetupAttachment(Root);
			BoxComp->RegisterComponent();

			// 启用组件复制，避免网络同步 warning
			BoxComp->SetIsReplicated(true);

			// 设置碰撞盒大小（半尺寸）
			BoxComp->SetBoxExtent(FVector(CellSize * 0.5f, CellSize * 0.5f, CellSize * 0.5f));

			// 计算碰撞盒位置（单元格中心，Z轴为高度的一半使底部与Grid平面对齐）
			float CellCenterX = (Col + 0.5f) * CellSize;
			float CellCenterY = (Row + 0.5f) * CellSize;
			FVector LocalPos = GridOrigin + CenterOffset + FVector(CellCenterX, CellCenterY, CellSize * 0.5f);
			BoxComp->SetRelativeLocation(LocalPos);

			// 设置碰撞属性 - 只阻挡 Pawn
			BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BoxComp->SetCollisionObjectType(ECC_WorldStatic);
			BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
			BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

			// 存储引用
			CellCollisions.Add(Key, BoxComp);
		}
	}
}

void ASHGridBase::OnRep_OccupiedCells()
{
	// 当 OccupiedCells 复制到客户端时，同步所有碰撞状态
	SyncAllCellCollisions();
}

void ASHGridBase::SyncAllCellCollisions()
{
	// 先禁用所有碰撞盒
	for (const auto& Pair : CellCollisions)
	{
		if (Pair.Value)
		{
			Pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// 根据 OccupiedCells 启用对应的碰撞盒
	for (const FIntPoint& Key : OccupiedCells)
	{
		UBoxComponent** Found = CellCollisions.Find(Key);
		if (Found && *Found)
		{
			(*Found)->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

void ASHGridBase::EnableCellCollisionLocal(const int32 Row, const int32 Column, const bool bEnable)
{
	const FIntPoint Key(Column, Row);
	UBoxComponent** Found = CellCollisions.Find(Key);
	if (Found && *Found)
	{
		(*Found)->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}

void ASHGridBase::MulticastEnableCellCollision_Implementation(const int32 Row, const int32 Column, const bool bEnable)
{
	EnableCellCollisionLocal(Row, Column, bEnable);
}

FVector ASHGridBase::GetMovementDirectionVector() const
{
	switch (MovementDirection)
	{
	case EGridMovementDirection::PositiveX:
		return FVector(1.0f, 0.0f, 0.0f);
	case EGridMovementDirection::NegativeX:
		return FVector(-1.0f, 0.0f, 0.0f);
	case EGridMovementDirection::PositiveY:
		return FVector(0.0f, 1.0f, 0.0f);
	case EGridMovementDirection::NegativeY:
		return FVector(0.0f, -1.0f, 0.0f);
	default:
		return FVector::ZeroVector;
	}
}

void ASHGridBase::SetMovementEnabled(bool bEnabled)
{
	if (HasAuthority())
	{
		bEnableMovement = bEnabled;
	}
}

void ASHGridBase::SetMovementSpeed(float NewSpeed)
{
	if (HasAuthority())
	{
		MovementSpeed = FMath::Max(0.0f, NewSpeed);
	}
}

void ASHGridBase::SetMovementDirection(EGridMovementDirection NewDirection)
{
	if (HasAuthority())
	{
		MovementDirection = NewDirection;
	}
}
