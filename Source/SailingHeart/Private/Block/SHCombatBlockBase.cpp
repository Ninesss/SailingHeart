// Sailing Heart

#include "Block/SHCombatBlockBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/AttributeSet/SHBlockAttributeSet.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "DrawDebugHelpers.h"

ASHCombatBlockBase::ASHCombatBlockBase()
{
	// 创建 AbilitySystemComponent
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	// 创建 AttributeSet
	AttributeSet = CreateDefaultSubobject<USHBlockAttributeSet>(TEXT("AttributeSet"));
}

void ASHCombatBlockBase::BeginPlay()
{
	Super::BeginPlay();

	// 如果 AttributeSet 为空，尝试从 ASC 获取
	if (!AttributeSet && AbilitySystemComponent)
	{
		AttributeSet = const_cast<USHBlockAttributeSet*>(AbilitySystemComponent->GetSet<USHBlockAttributeSet>());
	}

	// 如果还是空，手动创建
	if (!AttributeSet && AbilitySystemComponent)
	{
		AttributeSet = NewObject<USHBlockAttributeSet>(this, USHBlockAttributeSet::StaticClass());
		AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
	}

	// 初始化 ASC
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 注册阵营标签
		if (FactionTag.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(FactionTag);
		}
	}

	// 绑定死亡事件
	if (AttributeSet)
	{
		AttributeSet->OnDeath.AddDynamic(this, &ASHCombatBlockBase::OnDeathCallback);
	}
}

void ASHCombatBlockBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ASHCombatBlockBase::GetBlockLevel() const
{
	return AttributeSet ? AttributeSet->GetLevel() : 1.f;
}

// 属性访问器
float ASHCombatBlockBase::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float ASHCombatBlockBase::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

float ASHCombatBlockBase::GetAttack() const
{
	return AttributeSet ? AttributeSet->GetAttack() : 0.f;
}

float ASHCombatBlockBase::GetDefence() const
{
	return AttributeSet ? AttributeSet->GetDefence() : 0.f;
}

// ISHCombatInterface 实现
bool ASHCombatBlockBase::IsDead_Implementation() const
{
	return GetHealth() <= 0.f;
}

void ASHCombatBlockBase::Die_Implementation()
{
	HandleDeath();
}

AActor* ASHCombatBlockBase::GetAvatar_Implementation()
{
	return this;
}

float ASHCombatBlockBase::GetCriticalRate() const
{
	return AttributeSet ? AttributeSet->GetCriticalRate() : 0.f;
}

float ASHCombatBlockBase::GetCriticalDamage() const
{
	return AttributeSet ? AttributeSet->GetCriticalDamage() : 150.f;
}

float ASHCombatBlockBase::GetActionSpeed() const
{
	return AttributeSet ? AttributeSet->GetActionSpeed() : 100.f;
}

float ASHCombatBlockBase::GetEnergy() const
{
	return AttributeSet ? AttributeSet->GetEnergy() : 0.f;
}

float ASHCombatBlockBase::GetMaxEnergy() const
{
	return AttributeSet ? AttributeSet->GetMaxEnergy() : 0.f;
}

float ASHCombatBlockBase::GetEnergyRegen() const
{
	return AttributeSet ? AttributeSet->GetEnergyRegen() : 0.f;
}

void ASHCombatBlockBase::InitializeBlock(const FBlockInitParams& Params)
{
	if (!HasAuthority())
	{
		return;
	}

	// 1. 初始化属性（包含血量处理）
	InitializeAttributes(Params);

	// 2. 授予能力
	if (Params.AbilityDataAssets.Num() > 0)
	{
		GrantAbilitiesFromData(Params.AbilityDataAssets, Params.Level);
	}

	// 3. 应用能量回复 GE（如果配置了且有能量系统）
	if (EnergyRegenEffectClass && AbilitySystemComponent && Params.MaxEnergy > 0.f)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EnergyRegenEffectClass, 1, Context);
		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}

	// 4. 启用碰撞
	EnableBlockCollision();
}

void ASHCombatBlockBase::InitializeAttributes(const FBlockInitParams& Params)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	// 确保 AttributeSet 存在
	if (!AttributeSet)
	{
		// 尝试从 ASC 获取
		AttributeSet = const_cast<USHBlockAttributeSet*>(AbilitySystemComponent->GetSet<USHBlockAttributeSet>());
	}

	if (!AttributeSet)
	{
		// 手动创建并添加到 ASC
		AttributeSet = NewObject<USHBlockAttributeSet>(this, USHBlockAttributeSet::StaticClass());
		AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
	}

	// 确保 ASC 已初始化
	if (!AbilitySystemComponent->AbilityActorInfo.IsValid() || !AbilitySystemComponent->AbilityActorInfo->AvatarActor.IsValid())
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 设置 AttributeSet 的值
	AttributeSet->SetLevel(static_cast<float>(Params.Level));
	AttributeSet->SetMaxHealth(Params.MaxHealth);
	AttributeSet->SetAttack(Params.Attack);
	AttributeSet->SetDefence(Params.Defence);
	AttributeSet->SetCriticalRate(Params.CriticalRate);
	AttributeSet->SetCriticalDamage(Params.CriticalDamage);
	AttributeSet->SetActionSpeed(Params.ActionSpeed);
	AttributeSet->SetMaxEnergy(Params.MaxEnergy);
	AttributeSet->SetEnergyRegen(Params.EnergyRegen);
	AttributeSet->SetEnergy(Params.MaxEnergy);  // 初始能量 = 最大能量

	// 处理当前血量（-1 表示满血，否则使用指定值）
	if (Params.CurrentHealth >= 0.f)
	{
		AttributeSet->SetHealth(FMath::Clamp(Params.CurrentHealth, 0.f, Params.MaxHealth));
	}
	else
	{
		AttributeSet->SetHealth(Params.MaxHealth);
	}
}

void ASHCombatBlockBase::GrantAbilitiesFromData(const TArray<USHAbilityDataBase*>& AbilityDataAssets, int32 Level)
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (USHAbilityDataBase* AbilityData : AbilityDataAssets)
	{
		if (AbilityData && AbilityData->AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityData->AbilityClass, Level);
			Spec.SourceObject = AbilityData;  // 传入 DataAsset，能力可以通过 GetAbilityData() 获取配置
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void ASHCombatBlockBase::ClearAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->ClearAllAbilities();
}

void ASHCombatBlockBase::OnDeathCallback(AActor* DeadActor)
{
	if (DeadActor == this)
	{
		HandleDeath();
	}
}

void ASHCombatBlockBase::HandleDeath_Implementation()
{
	// 防止重复调用
	if (bIsDying)
	{
		return;
	}
	bIsDying = true;

	Destroy();
}

void ASHCombatBlockBase::DrawDebugInfo()
{
	FVector Location = GetActorLocation() + FVector(0, 0, 150.f);

	// 基础信息
	FString DebugText = FString::Printf(
		TEXT("%s\nLv: %.0f  HP: %.0f/%.0f"),
		*BlockTypeID.ToString(),
		GetBlockLevel(),
		GetHealth(),
		GetMaxHealth()
	);

	// 如果有能量系统，显示能量（和HP同一行）
	if (GetMaxEnergy() > 0.f)
	{
		DebugText += FString::Printf(TEXT("  Energy: %.0f/%.0f (+%.1f/s)"),
			GetEnergy(), GetMaxEnergy(), GetEnergyRegen());
	}

	// 战斗属性
	DebugText += FString::Printf(
		TEXT("\nATK: %.0f  DEF: %.0f%%\nCRIT: %.0f%%  CDMG: %.0f%%\nASPD: %.0f%%"),
		GetAttack(),
		GetDefence(),
		GetCriticalRate(),
		GetCriticalDamage(),
		GetActionSpeed()
	);

	// 子类额外信息
	FString ExtraText = GetExtraDebugText();
	if (!ExtraText.IsEmpty())
	{
		DebugText += TEXT("\n") + ExtraText;
	}

	// 技能列表
	if (AbilitySystemComponent)
	{
		TArray<FGameplayAbilitySpec>& Abilities = AbilitySystemComponent->GetActivatableAbilities();
		if (Abilities.Num() > 0)
		{
			DebugText += TEXT("\nAbilities:");
			for (const FGameplayAbilitySpec& Spec : Abilities)
			{
				if (Spec.Ability)
				{
					DebugText += FString::Printf(TEXT("\n  - %s"), *Spec.Ability->GetClass()->GetName());
				}
			}
		}
		else
		{
			DebugText += TEXT("\nAbilities: None");
		}
	}

	// StateTree 信息
	DebugText += TEXT("\n") + GetStateTreeDebugText();

	DrawDebugString(GetWorld(), Location, DebugText, nullptr, GetDebugColor(), 0.f, false, 1.0f);
}
