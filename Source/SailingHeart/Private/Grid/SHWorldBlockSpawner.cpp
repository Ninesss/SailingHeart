// Sailing Heart

#include "Grid/SHWorldBlockSpawner.h"
#include "Grid/SHGridBase.h"
#include "Block/SHEnemyBlock.h"
#include "Block/SHNeutralBlock.h"
#include "Data/SHEnemyBlockData.h"
#include "Data/SHNeutralBlockData.h"
#include "Game/SHGameStateBase.h"
#include "TimerManager.h"

ASHWorldBlockSpawner::ASHWorldBlockSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ASHWorldBlockSpawner::BeginPlay()
{
	Super::BeginPlay();

	CachedGameState = GetWorld()->GetGameState<ASHGameStateBase>();

	if (HasAuthority() && bAutoStart)
	{
		StartSpawning();
	}
}

void ASHWorldBlockSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void ASHWorldBlockSpawner::StartSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	ASHGridBase* TargetGrid = GetTargetGrid();
	if (!TargetGrid)
	{
		return;
	}

	// 检查是否至少有一种方块可生成
	if (EnemyBlockDataList.IsEmpty() && NeutralBlockDataList.IsEmpty())
	{
		return;
	}

	bIsSpawning = true;

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ASHWorldBlockSpawner::SpawnWave,
		SpawnInterval,
		true
	);
}

void ASHWorldBlockSpawner::StopSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsSpawning = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void ASHWorldBlockSpawner::SpawnWave()
{
	if (!HasAuthority())
	{
		return;
	}

	ASHGridBase* TargetGrid = GetTargetGrid();
	if (!TargetGrid)
	{
		return;
	}

	const int32 SpawnCount = FMath::RandRange(MinSpawnCount, MaxSpawnCount);
	TArray<int32> SelectedOffsets = SelectRandomLateralOffsets(SpawnCount);

	for (int32 Offset : SelectedOffsets)
	{
		FVector SpawnLocation = CalculateSpawnLocation(Offset);
		SpawnSingleBlock(SpawnLocation);
	}
}

void ASHWorldBlockSpawner::SpawnSingleBlock(const FVector& Location)
{
	const bool bHasEnemy = !EnemyBlockDataList.IsEmpty();
	const bool bHasNeutral = !NeutralBlockDataList.IsEmpty();

	if (!bHasEnemy && !bHasNeutral)
	{
		return;
	}

	// 根据权重决定生成类型
	bool bSpawnNeutral = false;

	if (bHasEnemy && bHasNeutral)
	{
		// 两种都有，按权重随机
		bSpawnNeutral = FMath::FRand() < NeutralSpawnWeight;
	}
	else if (bHasNeutral)
	{
		// 只有中立方块
		bSpawnNeutral = true;
	}
	// else: 只有敌人方块，bSpawnNeutral 保持 false

	if (bSpawnNeutral)
	{
		SpawnNeutralBlock(Location);
	}
	else
	{
		SpawnEnemyBlock(Location);
	}
}

void ASHWorldBlockSpawner::SpawnEnemyBlock(const FVector& Location)
{
	USHEnemyBlockData* EnemyData = GetRandomEnemyData();
	if (!EnemyData)
	{
		return;
	}

	int32 SpawnLevel = FMath::RandRange(MinEnemyLevel, MaxEnemyLevel);
	ASHEnemyBlock::SpawnDeferred(GetWorld(), EnemyData, Location, SpawnLevel, GetFacingRotation());
}

void ASHWorldBlockSpawner::SpawnNeutralBlock(const FVector& Location)
{
	USHNeutralBlockData* NeutralData = GetRandomNeutralData();
	if (!NeutralData)
	{
		return;
	}

	int32 SpawnLevel = FMath::RandRange(MinNeutralLevel, MaxNeutralLevel);
	ASHNeutralBlock::SpawnDeferred(GetWorld(), NeutralData, Location, SpawnLevel);
}

FRotator ASHWorldBlockSpawner::GetFacingRotation() const
{
	// 方块朝向应与 SpawnDirection 相反（朝向网格）
	// UE Yaw: 0=+X, 90=+Y, 180=-X, 270(-90)=-Y
	switch (SpawnDirection)
	{
	case EGridMovementDirection::PositiveX:  return FRotator(0.f, 180.f, 0.f);
	case EGridMovementDirection::NegativeX:  return FRotator(0.f,   0.f, 0.f);
	case EGridMovementDirection::PositiveY:  return FRotator(0.f, 270.f, 0.f);
	case EGridMovementDirection::NegativeY:  return FRotator(0.f,  90.f, 0.f);
	default:                                 return FRotator::ZeroRotator;
	}
}

ASHGridBase* ASHWorldBlockSpawner::GetTargetGrid()
{
	if (CachedTargetGrid)
	{
		return CachedTargetGrid;
	}

	if (!CachedGameState)
	{
		CachedGameState = GetWorld()->GetGameState<ASHGameStateBase>();
	}

	if (CachedGameState)
	{
		CachedTargetGrid = CachedGameState->GetPlayerGrid();
	}

	return CachedTargetGrid;
}

USHEnemyBlockData* ASHWorldBlockSpawner::GetRandomEnemyData()
{
	// 收集有效条目并累计总权重
	float TotalWeight = 0.f;
	TArray<const FSHEnemyBlockEntry*> ValidEntries;
	for (const FSHEnemyBlockEntry& Entry : EnemyBlockDataList)
	{
		if (Entry.BlockData && Entry.BlockData->EnemyClass && Entry.Weight > 0.f)
		{
			ValidEntries.Add(&Entry);
			TotalWeight += Entry.Weight;
		}
	}

	if (ValidEntries.IsEmpty() || TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Roll = FMath::FRand() * TotalWeight;
	for (const FSHEnemyBlockEntry* Entry : ValidEntries)
	{
		Roll -= Entry->Weight;
		if (Roll <= 0.f)
		{
			return Entry->BlockData;
		}
	}

	return ValidEntries.Last()->BlockData;
}

USHNeutralBlockData* ASHWorldBlockSpawner::GetRandomNeutralData()
{
	// 收集有效条目并累计总权重
	float TotalWeight = 0.f;
	TArray<const FSHNeutralBlockEntry*> ValidEntries;
	for (const FSHNeutralBlockEntry& Entry : NeutralBlockDataList)
	{
		if (Entry.BlockData && Entry.BlockData->NeutralClass && Entry.Weight > 0.f)
		{
			ValidEntries.Add(&Entry);
			TotalWeight += Entry.Weight;
		}
	}

	if (ValidEntries.IsEmpty() || TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Roll = FMath::FRand() * TotalWeight;
	for (const FSHNeutralBlockEntry* Entry : ValidEntries)
	{
		Roll -= Entry->Weight;
		if (Roll <= 0.f)
		{
			return Entry->BlockData;
		}
	}

	return ValidEntries.Last()->BlockData;
}

TArray<int32> ASHWorldBlockSpawner::SelectRandomLateralOffsets(int32 Count)
{
	TArray<int32> Available;
	TArray<int32> Selected;

	for (int32 i = MinLateralOffset; i <= MaxLateralOffset; i++)
	{
		Available.Add(i);
	}

	Count = FMath::Min(Count, Available.Num());

	for (int32 i = 0; i < Count; i++)
	{
		if (Available.Num() == 0)
		{
			break;
		}

		const int32 RandomIndex = FMath::RandRange(0, Available.Num() - 1);
		Selected.Add(Available[RandomIndex]);
		Available.RemoveAt(RandomIndex);
	}

	return Selected;
}

FVector ASHWorldBlockSpawner::CalculateSpawnLocation(int32 LateralOffset)
{
	ASHGridBase* TargetGrid = GetTargetGrid();
	if (!TargetGrid)
	{
		return FVector::ZeroVector;
	}

	const float CellSize = TargetGrid->CellSize;
	const int32 GridRows = TargetGrid->Rows;
	const int32 GridColumns = TargetGrid->Columns;
	const FVector GridLocation = TargetGrid->GetActorLocation();
	const FVector GridOrigin = TargetGrid->GridOrigin;

	const int32 TotalRows = GridRows + 2;
	const int32 TotalColumns = GridColumns + 2;

	// Row → X 轴，Column → Y 轴
	const FVector CenterOffset(-TotalRows * CellSize * 0.5f, -TotalColumns * CellSize * 0.5f, 0.0f);
	const FVector GridBottomLeft = GridLocation + GridOrigin + CenterOffset;

	// Grid 中心格子索引（X → Row，Y → Column）
	const float GridCenterRow = TotalRows * 0.5f;
	const float GridCenterCol = TotalColumns * 0.5f;

	float SpawnX = 0.0f;
	float SpawnY = 0.0f;

	switch (SpawnDirection)
	{
	case EGridMovementDirection::PositiveX:
		// 前进方向 +X（Row 增大），横向偏移沿 Y（Column）
		SpawnX = (TotalRows + SpawnDistanceOffset + 0.5f) * CellSize;
		SpawnY = (GridCenterCol + LateralOffset + 0.5f) * CellSize;
		break;

	case EGridMovementDirection::NegativeX:
		SpawnX = (-SpawnDistanceOffset - 0.5f) * CellSize;
		SpawnY = (GridCenterCol + LateralOffset + 0.5f) * CellSize;
		break;

	case EGridMovementDirection::PositiveY:
		SpawnX = (GridCenterRow + LateralOffset + 0.5f) * CellSize;
		SpawnY = (TotalColumns + SpawnDistanceOffset + 0.5f) * CellSize;
		break;

	case EGridMovementDirection::NegativeY:
		SpawnX = (GridCenterRow + LateralOffset + 0.5f) * CellSize;
		SpawnY = (-SpawnDistanceOffset - 0.5f) * CellSize;
		break;

	case EGridMovementDirection::None:
	default:
		SpawnX = (TotalRows + SpawnDistanceOffset + 0.5f) * CellSize;
		SpawnY = (GridCenterCol + LateralOffset + 0.5f) * CellSize;
		break;
	}

	return GridBottomLeft + FVector(SpawnX, SpawnY, 0.0f);
}
