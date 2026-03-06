// Sailing Heart

#include "Block/SHEnemyBlock.h"
#include "Data/SHEnemyBlockData.h"
#include "Components/StateTreeComponent.h"
#include "SHGameplayTags.h"

ASHEnemyBlock::ASHEnemyBlock()
{
	// 设置阵营标签
	FactionTag = FSHGameplayTags::Get().Faction_Enemy;

	// 设置碰撞预设（在基类配置中设置 CellSize）
	BlockCollisionProfileName = TEXT("EnemyBlock");
	SetupBlockCollision();
}

ASHEnemyBlock* ASHEnemyBlock::SpawnDeferred(
	UWorld* World,
	USHEnemyBlockData* EnemyData,
	const FVector& Location,
	int32 Level)
{
	if (!World || !EnemyData || !EnemyData->EnemyClass)
	{
		return nullptr;
	}

	FTransform SpawnTransform(FRotator::ZeroRotator, Location);

	// 使用 SpawnActorDeferred 生成方块
	ASHEnemyBlock* NewBlock = World->SpawnActorDeferred<ASHEnemyBlock>(
		EnemyData->EnemyClass,
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
	NewBlock->BlockTypeID = EnemyData->EnemyTypeID;

	// 获取等级配置
	FBlockLevelConfig LevelConfig = EnemyData->GetLevelConfig(Level);

	// 构建初始化参数
	FBlockInitParams Params;
	Params.Level = Level;
	Params.MaxHealth = LevelConfig.MaxHealth;
	Params.Attack = LevelConfig.Attack;
	Params.Defence = LevelConfig.Defence;
	Params.CriticalRate = LevelConfig.CriticalRate;
	Params.CriticalDamage = LevelConfig.CriticalDamage;
	Params.ActionSpeed = LevelConfig.ActionSpeed;
	Params.MaxEnergy = LevelConfig.MaxEnergy;
	Params.EnergyRegen = LevelConfig.EnergyRegen;
	Params.AbilityDataAssets = LevelConfig.GrantedAbilities;

	// 在 FinishSpawning 之前初始化属性
	NewBlock->InitializeBlock(Params);

	// 设置 StateTree
	if (EnemyData->StateTree && NewBlock->StateTreeComponent)
	{
		NewBlock->StateTreeComponent->SetStateTree(EnemyData->StateTree);
	}

	// 完成生成
	NewBlock->FinishSpawning(SpawnTransform);

	// 启动 StateTree（在 FinishSpawning 之后，只在服务器运行）
	if (NewBlock->HasAuthority() && EnemyData->StateTree && NewBlock->StateTreeComponent)
	{
		NewBlock->StateTreeComponent->StartLogic();
	}

	return NewBlock;
}
