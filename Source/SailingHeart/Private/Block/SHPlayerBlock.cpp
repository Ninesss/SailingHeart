// Sailing Heart

#include "Block/SHPlayerBlock.h"
#include "Block/SHCombatBlockBase.h"
#include "Grid/SHGridBase.h"
#include "Data/SHPlayerBlockData.h"
#include "SHGameplayTags.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/SHBlockAttributeSet.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "Net/UnrealNetwork.h"

ASHPlayerBlock::ASHPlayerBlock()
{
	// 默认启用移动复制
	SetReplicateMovement(true);

	// 设置阵营标签
	FactionTag = FSHGameplayTags::Get().Faction_Player;

	// 设置碰撞预设（在基类配置中设置 CellSize）
	BlockCollisionProfileName = TEXT("PlayerBlock");
	SetupBlockCollision();
}

ASHPlayerBlock* ASHPlayerBlock::SpawnDeferred(
	UWorld* World,
	TSubclassOf<ASHPlayerBlock> BlockClass,
	ASHGridBase* Grid,
	int32 Row, int32 Column,
	FName BlockTypeID,
	int32 Level,
	const FBlockLevelConfig& LevelConfig,
	float CurrentHealth)
{
	if (!World || !BlockClass || !Grid)
	{
		return nullptr;
	}

	FVector Location = Grid->GetCellCenterLocation(Row, Column);
	FTransform SpawnTransform(FRotator::ZeroRotator, Location);

	// 使用 SpawnActorDeferred 生成方块
	ASHPlayerBlock* NewBlock = World->SpawnActorDeferred<ASHPlayerBlock>(
		BlockClass,
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
	NewBlock->SetCell(Row, Column);
	NewBlock->SetOwnerGrid(Grid);
	NewBlock->BlockTypeID = BlockTypeID;

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
	Params.CurrentHealth = CurrentHealth;
	Params.AbilityDataAssets = LevelConfig.GrantedAbilities;

	// 在 FinishSpawning 之前初始化属性（这样碰撞事件触发时属性已就绪）
	NewBlock->InitializeBlock(Params);

	// 注册到 Grid（这样碰撞事件触发时 HasShipBlockAt 返回 true）
	Grid->SetShipBlockAt(Row, Column, NewBlock);

	// 完成生成（此时执行 BeginPlay，可能触发碰撞事件，属性已正确设置）
	NewBlock->FinishSpawning(SpawnTransform);

	return NewBlock;
}

void ASHPlayerBlock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHPlayerBlock, Cell);
}

void ASHPlayerBlock::BeginPlay()
{
	Super::BeginPlay();

	// 绑定碰撞事件（只在服务器处理）
	if (HasAuthority() && BlockCollisionBox)
	{
		BlockCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASHPlayerBlock::OnBlockCollisionBeginOverlap);
	}
}

void ASHPlayerBlock::OnBlockCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 只在服务器处理
	if (!HasAuthority())
	{
		return;
	}

	// 使用阵营标签判断是否为敌人
	if (!USHAbilitySystemLibrary::AreActorsEnemies(this, OtherActor))
	{
		return;
	}

	// 检查是否是战斗方块（需要有 ASC）
	ASHCombatBlockBase* EnemyBlock = Cast<ASHCombatBlockBase>(OtherActor);
	if (!EnemyBlock)
	{
		return;
	}

	// 清理无效的弱引用
	for (auto It = ProcessedCollisions.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	// 检查是否已处理过这个碰撞
	if (ProcessedCollisions.Contains(OtherActor))
	{
		return;
	}

	// 标记为已处理
	ProcessedCollisions.Add(OtherActor);

	// 获取双方血量
	const float MyHealth = GetHealth();
	const float EnemyHealth = EnemyBlock->GetHealth();

	// 应用碰撞伤害（双方互相造成伤害）
	if (CollisionDamageEffect)
	{
		UAbilitySystemComponent* MyASC = AbilitySystemComponent;
		UAbilitySystemComponent* EnemyASC = EnemyBlock->GetAbilitySystemComponent();

		if (MyASC && EnemyASC)
		{
			// 我对敌人造成伤害（伤害值 = 我的剩余血量）
			USHAbilitySystemLibrary::ApplyCollisionDamage(MyASC, EnemyASC, CollisionDamageEffect, MyHealth);

			// 敌人对我造成伤害（伤害值 = 敌人的剩余血量）
			USHAbilitySystemLibrary::ApplyCollisionDamage(EnemyASC, MyASC, CollisionDamageEffect, EnemyHealth);
		}
	}
}

void ASHPlayerBlock::HandleDeath_Implementation()
{
	// 防止重复调用
	if (bIsDying)
	{
		return;
	}
	bIsDying = true;

	// 清理碰撞记录
	ProcessedCollisions.Empty();

	// 从 Grid 中清理数据
	if (ASHGridBase* Grid = OwnerGrid.Get())
	{
		Grid->ClearShipBlockAt(Cell.Row, Cell.Column);
	}

	// 调用基类处理（Destroy）- 基类会跳过因为 bIsDying 已设置
	Destroy();
}

bool ASHPlayerBlock::UpgradeToLevel(int32 NewLevel, const FBlockLevelConfig& NewLevelConfig, bool bRestoreFullHealth)
{
	if (!HasAuthority())
	{
		return false;
	}

	// 记录当前血量比例（用于非满血升级）
	const float HealthRatio = GetMaxHealth() > 0.f ? GetHealth() / GetMaxHealth() : 1.f;

	// 清除旧能力
	ClearAbilities();

	// 构建新等级的初始化参数
	FBlockInitParams Params;
	Params.Level = NewLevel;
	Params.MaxHealth = NewLevelConfig.MaxHealth;
	Params.Attack = NewLevelConfig.Attack;
	Params.Defence = NewLevelConfig.Defence;
	Params.CriticalRate = NewLevelConfig.CriticalRate;
	Params.CriticalDamage = NewLevelConfig.CriticalDamage;
	Params.ActionSpeed = NewLevelConfig.ActionSpeed;
	Params.MaxEnergy = NewLevelConfig.MaxEnergy;
	Params.EnergyRegen = NewLevelConfig.EnergyRegen;

	// 根据参数决定血量
	if (bRestoreFullHealth)
	{
		Params.CurrentHealth = -1.f;  // 满血
	}
	else
	{
		// 保持血量比例
		Params.CurrentHealth = NewLevelConfig.MaxHealth * HealthRatio;
	}

	// 重新初始化属性（碰撞已启用所以不会重复）
	InitializeAttributes(Params);
	GrantAbilitiesFromData(NewLevelConfig.GrantedAbilities, NewLevel);

	// 清理碰撞记录（新等级重新开始）
	ProcessedCollisions.Empty();

	return true;
}

bool ASHPlayerBlock::MergeWith(const FBlockCarryState& OtherBlockState, const FBlockLevelConfig& NewLevelConfig)
{
	if (!HasAuthority())
	{
		return false;
	}

	// 检查类型和等级是否相同
	const int32 CurrentLevel = FMath::RoundToInt(GetBlockLevel());
	if (BlockTypeID != OtherBlockState.BlockTypeID || CurrentLevel != OtherBlockState.Level)
	{
		return false;
	}

	// 合成升级，回满血
	return UpgradeToLevel(CurrentLevel + 1, NewLevelConfig, true);
}

FBlockCarryState ASHPlayerBlock::CreateCarryState() const
{
	FBlockCarryState State;
	State.BlockTypeID = BlockTypeID;

	if (AttributeSet)
	{
		State.Level = FMath::RoundToInt(AttributeSet->GetLevel());
		State.CurrentHealth = AttributeSet->GetHealth();
	}

	return State;
}

void ASHPlayerBlock::SetCell(int32 Row, int32 Column)
{
	if (!HasAuthority())
	{
		return;
	}

	Cell = FGridCell(Row, Column);
}

void ASHPlayerBlock::SetOwnerGrid(ASHGridBase* Grid)
{
	OwnerGrid = Grid;
}

void ASHPlayerBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FString ASHPlayerBlock::GetExtraDebugText() const
{
	return FString::Printf(TEXT("Cell: %s"), *Cell.ToString());
}
