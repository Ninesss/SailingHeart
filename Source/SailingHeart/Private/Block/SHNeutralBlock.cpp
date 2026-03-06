// Sailing Heart

#include "Block/SHNeutralBlock.h"
#include "Block/SHPlayerBlock.h"
#include "Grid/SHGridBase.h"
#include "Data/SHNeutralBlockData.h"
#include "Data/SHPlayerBlockData.h"
#include "Game/SHGameStateBase.h"
#include "Components/StateTreeComponent.h"
#include "SHGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

ASHNeutralBlock::ASHNeutralBlock()
{
	// 设置阵营标签
	FactionTag = FSHGameplayTags::Get().Faction_Neutral;

	// 设置碰撞预设
	BlockCollisionProfileName = TEXT("NeutralBlock");
	SetupBlockCollision();
}

void ASHNeutralBlock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHNeutralBlock, TargetBlockTypeID);
	DOREPLIFETIME(ASHNeutralBlock, Level);
}

void ASHNeutralBlock::BeginPlay()
{
	Super::BeginPlay();

	// 绑定碰撞事件（只在服务器处理）
	if (HasAuthority() && BlockCollisionBox)
	{
		BlockCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASHNeutralBlock::OnNeutralBlockBeginOverlap);
	}
}

ASHNeutralBlock* ASHNeutralBlock::SpawnDeferred(
	UWorld* World,
	USHNeutralBlockData* NeutralData,
	const FVector& Location,
	int32 InLevel)
{
	if (!World || !NeutralData || !NeutralData->NeutralClass)
	{
		return nullptr;
	}

	FTransform SpawnTransform(FRotator::ZeroRotator, Location);

	// 使用 SpawnActorDeferred 生成方块
	ASHNeutralBlock* NewBlock = World->SpawnActorDeferred<ASHNeutralBlock>(
		NeutralData->NeutralClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!NewBlock)
	{
		return nullptr;
	}

	// 在 FinishSpawning 之前设置所有属性
	NewBlock->BlockTypeID = NeutralData->NeutralTypeID;
	NewBlock->TargetBlockTypeID = NeutralData->TargetBlockTypeID;
	NewBlock->Level = InLevel;

	// 设置 StateTree
	if (NeutralData->StateTree && NewBlock->StateTreeComponent)
	{
		NewBlock->StateTreeComponent->SetStateTree(NeutralData->StateTree);
	}

	// 启用碰撞
	NewBlock->EnableBlockCollision();

	// 完成生成
	NewBlock->FinishSpawning(SpawnTransform);

	// 启动 StateTree（在 FinishSpawning 之后，只在服务器运行）
	if (NewBlock->HasAuthority() && NeutralData->StateTree && NewBlock->StateTreeComponent)
	{
		NewBlock->StateTreeComponent->StartLogic();
	}

	return NewBlock;
}

void ASHNeutralBlock::OnNeutralBlockBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 只在服务器处理
	if (!HasAuthority())
	{
		return;
	}

	// 检查碰撞对象是否是玩家方块
	ASHPlayerBlock* PlayerBlock = Cast<ASHPlayerBlock>(OtherActor);
	if (!PlayerBlock)
	{
		return;
	}

	// 获取玩家方块的 Grid
	ASHGridBase* PlayerGrid = PlayerBlock->GetOwnerGrid();
	if (!PlayerGrid)
	{
		return;
	}

	// 禁用碰撞，防止 FinishSpawning 时再次触发
	if (BlockCollisionBox)
	{
		BlockCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 尝试生成玩家方块
	TrySpawnPlayerBlock(PlayerGrid);

	// 无论是否成功，都销毁自己
	Destroy();
}

bool ASHNeutralBlock::TrySpawnPlayerBlock(ASHGridBase* PlayerGrid)
{
	if (!PlayerGrid)
	{
		return false;
	}

	// 计算自己在 Grid 中的位置
	int32 Row, Column;
	if (!PlayerGrid->GetCellIndexFromLocation(GetActorLocation(), Row, Column))
	{
		return false;
	}

	// 检查单元格是否可放置
	if (!PlayerGrid->IsCellPlaceable(Row, Column))
	{
		return false;
	}

	// 检查单元格是否已有方块
	if (PlayerGrid->HasShipBlockAt(Row, Column))
	{
		return false;
	}

	// 获取 GameState 和玩家方块数据
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	ASHGameStateBase* GS = World->GetGameState<ASHGameStateBase>();
	if (!GS)
	{
		return false;
	}

	USHPlayerBlockData* BlockData = GS->GetPlayerBlockData(TargetBlockTypeID);
	if (!BlockData || !BlockData->BlockClass)
	{
		return false;
	}

	// 检查等级是否有效
	if (!BlockData->IsLevelValid(Level))
	{
		return false;
	}

	// 获取等级配置
	FBlockLevelConfig LevelConfig = BlockData->GetLevelConfig(Level);

	// 使用统一的 Deferred 生成方法
	ASHPlayerBlock* NewBlock = ASHPlayerBlock::SpawnDeferred(
		World,
		BlockData->BlockClass,
		PlayerGrid,
		Row, Column,
		TargetBlockTypeID,
		Level,
		LevelConfig,
		-1.f,
		BlockData->StateTree
	);

	return NewBlock != nullptr;
}

void ASHNeutralBlock::DrawDebugInfo()
{
	FVector Location = GetActorLocation() + FVector(0, 0, 150.f);

	FString DebugText = FString::Printf(
		TEXT("%s\nTarget: %s\nLevel: %d\n%s"),
		*BlockTypeID.ToString(),
		*TargetBlockTypeID.ToString(),
		Level,
		*GetStateTreeDebugText()
	);

	DrawDebugString(GetWorld(), Location, DebugText, nullptr, FColor::Cyan, 0.f, false, 1.0f);
}
