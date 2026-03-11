// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/SHGameplayAbilityBase.h"
#include "AbilitySystem/Ability/SHDamageAbilityInterface.h"
#include "SHGameplayAbility_Melee.generated.h"

/**
 * 近战攻击技能 - 占位类，具体实现待补全
 *
 * 预期流程（与 Projectile 相同的两段式）：
 * 1. ActivateAbility：缓存目标 → 播放攻击 Montage
 * 2. USHAbilityAnimNotify 在攻击帧触发 GameplayEvent
 * 3. OnAnimNotifyEvent：在攻击范围内对目标施加 GameplayEffect
 */
UCLASS(Abstract)
class SAILINGHEART_API USHGameplayAbility_Melee
	: public USHGameplayAbilityBase
	, public ISHDamageAbilityInterface
{
	GENERATED_BODY()

public:
	USHGameplayAbility_Melee();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// ISHDamageAbilityInterface 实现（占位，子类填入）
	virtual TSubclassOf<UGameplayEffect> GetDamageEffectClass_Implementation() const override { return nullptr; }
	virtual FGameplayTagContainer GetDamageTypeTags_Implementation() const override { return FGameplayTagContainer(); }
	virtual float GetDamageMultiplier_Implementation() const override { return 1.f; }
	virtual UAbilitySystemComponent* GetDamageSourceASC_Implementation() const override;
	virtual AActor* GetDamageSourceActor_Implementation() const override;
};
