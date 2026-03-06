// Sailing Heart

#include "AbilitySystem/Ability/SHGameplayAbility_Toggle.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Data/Ability/SHAbilityParams.h"
#include "Data/Ability/SHAbilityDataBase.h"

USHGameplayAbility_Toggle::USHGameplayAbility_Toggle()
{
	// 使用 InstancedPerActor，每个 Actor 一个实例
	// 这样可以在 OnRemoveAbility 中清理状态（如角色死亡时）
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 服务器执行
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void USHGameplayAbility_Toggle::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// 如果能力被移除时还处于激活状态，强制清理
	// 这会在 ASC 销毁时触发（如角色死亡）
	if (IsToggleActive())
	{
		// 停止能量消耗
		StopEnergyConsumption();

		// 调用子类的清理逻辑（如关闭时间减缓）
		OnToggleOff();

		// 移除激活状态 Tag
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayTag ActiveTag = GetToggleActiveTag();
			if (ActiveTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(ActiveTag);
			}
		}
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool USHGameplayAbility_Toggle::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 如果已激活，检查是否可以关闭
	if (IsToggleActive())
	{
		return CanToggleOff();
	}

	// 检查冷却
	if (IsToggleOnCooldown())
	{
		return false;
	}

	// 检查是否可以激活
	return CanToggleOn();
}

void USHGameplayAbility_Toggle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Toggle 逻辑
	if (IsToggleActive())
	{
		DeactivateToggle();
	}
	else
	{
		ActivateToggle();
	}

	// 立即结束能力（Toggle 能力不持续，状态由 Tag 管理）
	K2_EndAbility();
}

void USHGameplayAbility_Toggle::ActivateToggle()
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	// 添加激活状态 Tag
	FGameplayTag ActiveTag = GetToggleActiveTag();
	if (ActiveTag.IsValid())
	{
		ASC->AddLooseGameplayTag(ActiveTag);
	}

	// 启动能量消耗（如果有）
	StartEnergyConsumption();

	// 调用子类实现
	OnToggleOn();
}

void USHGameplayAbility_Toggle::DeactivateToggle()
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	// 停止能量消耗
	StopEnergyConsumption();

	// 移除激活状态 Tag
	FGameplayTag ActiveTag = GetToggleActiveTag();
	if (ActiveTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(ActiveTag);
	}

	// 调用子类实现
	OnToggleOff();

	// 关闭时必定进入冷却
	ApplyToggleCooldown();
}

void USHGameplayAbility_Toggle::ForceDeactivateToggle()
{
	// 复用 DeactivateToggle 逻辑，避免代码重复
	DeactivateToggle();
}

bool USHGameplayAbility_Toggle::CanToggleOn() const
{
	// 检查能量是否足够启动
	return HasEnoughEnergyToContinue();
}

FGameplayTag USHGameplayAbility_Toggle::GetToggleActiveTag() const
{
	// 从DataAsset读取ActiveTag
	if (const FSHToggleAbilityParams* Params = GetToggleParams())
	{
		return Params->ActiveTag;
	}
	return FGameplayTag();
}

bool USHGameplayAbility_Toggle::IsToggleActive() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(GetToggleActiveTag());
}

bool USHGameplayAbility_Toggle::IsToggleOnCooldown() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(GetToggleCooldownTag());
}

FGameplayTag USHGameplayAbility_Toggle::GetToggleCooldownTag() const
{
	// 使用基类的CooldownTag生成逻辑（基于AbilityID）
	// 格式: Cooldown.Ability.{AbilityID}
	return GetAbilityCooldownTag();
}

// ========== 参数获取（从子类获取）==========

float USHGameplayAbility_Toggle::GetCooldownDuration() const
{
	if (const FSHToggleAbilityParams* Params = GetToggleParams())
	{
		return Params->CooldownDuration;
	}
	return 0.5f;
}

float USHGameplayAbility_Toggle::GetEnergyCostPerSecond() const
{
	if (const FSHToggleAbilityParams* Params = GetToggleParams())
	{
		return Params->EnergyCostPerSecond;
	}
	return 0.f;
}

void USHGameplayAbility_Toggle::ApplyToggleCooldown()
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	FGameplayTag CDTag = GetToggleCooldownTag();
	if (!CDTag.IsValid())
	{
		return;
	}

	// 添加冷却 Tag
	ASC->AddLooseGameplayTag(CDTag);

	// 获取冷却时间（从 DataAsset 或默认值）
	float CDDuration = GetCooldownDuration();

	// 设置定时器移除冷却 Tag
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle TimerHandle;
		TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
		FGameplayTag CapturedTag = CDTag;

		World->GetTimerManager().SetTimer(
			TimerHandle,
			FTimerDelegate::CreateWeakLambda(ASC, [WeakASC, CapturedTag]()
			{
				if (UAbilitySystemComponent* ASCLocal = WeakASC.Get())
				{
					ASCLocal->RemoveLooseGameplayTag(CapturedTag);
				}
			}),
			CDDuration,
			false  // 不循环
		);
	}
}

// ========== 持续能量消耗 ==========

void USHGameplayAbility_Toggle::StartEnergyConsumption()
{
	float CostPerSecond = GetEnergyCostPerSecond();
	if (CostPerSecond <= 0.f)
	{
		return;  // 无持续消耗
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	// 启动能量消耗定时器（使用 Lambda 绑定到 ASC，避免 Ability 结束后 Timer 失效）
	TWeakObjectPtr<USHGameplayAbility_Toggle> WeakThis = this;
	World->GetTimerManager().SetTimer(
		EnergyConsumeTimerHandle,
		FTimerDelegate::CreateWeakLambda(ASC, [WeakThis]()
		{
			if (USHGameplayAbility_Toggle* Ability = WeakThis.Get())
			{
				Ability->ConsumeEnergyTick();
			}
		}),
		EnergyConsumeInterval,
		true  // 循环
	);

	// 监听能量属性变化
	ASC->GetGameplayAttributeValueChangeDelegate(
		USHAttributeSetBase::GetEnergyAttribute()
	).AddUObject(this, &USHGameplayAbility_Toggle::OnEnergyAttributeChanged);
}

void USHGameplayAbility_Toggle::StopEnergyConsumption()
{
	// 停止定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EnergyConsumeTimerHandle);
	}

	// 移除属性监听
	if (UAbilitySystemComponent* ASC = GetOwnerASC())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			USHAttributeSetBase::GetEnergyAttribute()
		).RemoveAll(this);
	}
}

void USHGameplayAbility_Toggle::ConsumeEnergyTick()
{
	float CostPerSecond = GetEnergyCostPerSecond();
	float CostThisTick = CostPerSecond * EnergyConsumeInterval;
	SpendEnergy(CostThisTick);
}

void USHGameplayAbility_Toggle::OnEnergyAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 检查能量是否不足以继续
	if (!HasEnoughEnergyToContinue() && IsToggleActive())
	{
		ForceDeactivateToggle();
	}
}

bool USHGameplayAbility_Toggle::HasEnoughEnergyToContinue() const
{
	float CostPerSecond = GetEnergyCostPerSecond();
	if (CostPerSecond <= 0.f)
	{
		return true;  // 无消耗，永远足够
	}

	float CurrentEnergy = GetCurrentEnergy();
	return CurrentEnergy > 0.f;  // 有能量就能继续，用到精光
}
