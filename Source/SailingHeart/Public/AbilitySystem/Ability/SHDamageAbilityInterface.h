// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "SHDamageAbilityInterface.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UINTERFACE(MinimalAPI, BlueprintType)
class USHDamageAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 伤害能力接口 - 实现此接口的能力可以造成伤害
 *
 * 实现要求：
 * - GetDamageEffectClass(): 返回伤害 GE 类
 * - GetDamageTypeTags(): 返回伤害类型（支持多个）
 * - GetDamageSourceASC(): 返回伤害来源的 ASC
 * - GetDamageSourceActor(): 返回伤害来源 Actor
 *
 * 接口提供默认实现：
 * - MakeDamageSpec(): 创建伤害 Spec
 * - ApplyDamageToTarget(): 应用伤害到目标
 * - ApplyDamageToTargets(): 应用伤害到多个目标
 */
class SAILINGHEART_API ISHDamageAbilityInterface
{
	GENERATED_BODY()

public:
	// ========== 子类必须实现（参数获取）==========

	// 获取伤害 GE 类
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	TSubclassOf<UGameplayEffect> GetDamageEffectClass() const;

	// 获取伤害类型（支持多个 Tag）
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	FGameplayTagContainer GetDamageTypeTags() const;

	// 获取伤害倍率（默认 1.0）
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	float GetDamageMultiplier() const;
	virtual float GetDamageMultiplier_Implementation() const { return 1.0f; }

	// ========== 子类必须实现（依赖获取）==========

	// 获取伤害来源的 ASC
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	UAbilitySystemComponent* GetDamageSourceASC() const;

	// 获取伤害来源 Actor
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	AActor* GetDamageSourceActor() const;

	// ========== 接口提供默认实现（伤害功能）==========

	// 创建伤害 Spec
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	FGameplayEffectSpecHandle MakeDamageSpec(float Level = 1.f) const;
	virtual FGameplayEffectSpecHandle MakeDamageSpec_Implementation(float Level = 1.f) const;

	// 应用伤害到单个目标
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	bool ApplyDamageToTarget(AActor* Target, float Level = 1.f);
	virtual bool ApplyDamageToTarget_Implementation(AActor* Target, float Level = 1.f);

	// 应用伤害到多个目标，返回成功数量
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	int32 ApplyDamageToTargets(const TArray<AActor*>& Targets, float Level = 1.f);
	virtual int32 ApplyDamageToTargets_Implementation(const TArray<AActor*>& Targets, float Level = 1.f);
};
