// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "SHAbilitySystemLibrary.generated.h"

struct FSHGameplayEffectContext;
class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * GAS 辅助函数库
 */
UCLASS()
class SAILINGHEART_API USHAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========== Context 辅助函数 ==========

	// 获取自定义 Context（只读版本）
	static const FSHGameplayEffectContext* GetSHEffectContextConst(const FGameplayEffectContextHandle& ContextHandle);

	// 获取自定义 Context（可修改版本）
	static FSHGameplayEffectContext* GetSHEffectContext(FGameplayEffectContextHandle& ContextHandle);

	// ========== 伤害类型（多 Tag 支持）==========

	// 设置伤害类型标签（多个）
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|AbilitySystem")
	static void SetDamageTypeTags(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, const FGameplayTagContainer& DamageTypeTags);

	// 获取伤害类型标签（多个）
	UFUNCTION(BlueprintPure, Category = "SailingHeart|AbilitySystem")
	static FGameplayTagContainer GetDamageTypeTags(const FGameplayEffectContextHandle& ContextHandle);

	// 添加单个伤害类型标签
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|AbilitySystem")
	static void AddDamageTypeTag(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, const FGameplayTag& DamageTypeTag);

	// 向后兼容：设置单个伤害类型标签（会清除已有的）
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|AbilitySystem")
	static void SetDamageTypeTag(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, const FGameplayTag& DamageTypeTag);

	// 向后兼容：获取第一个伤害类型标签
	UFUNCTION(BlueprintPure, Category = "SailingHeart|AbilitySystem")
	static FGameplayTag GetDamageTypeTag(const FGameplayEffectContextHandle& ContextHandle);

	// 是否为碰撞伤害
	UFUNCTION(BlueprintPure, Category = "SailingHeart|AbilitySystem")
	static bool IsCollisionDamage(const FGameplayEffectContextHandle& ContextHandle);

	// ========== 暴击 ==========

	// 设置暴击
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|AbilitySystem")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, bool bIsCriticalHit);

	// 是否暴击
	UFUNCTION(BlueprintPure, Category = "SailingHeart|AbilitySystem")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& ContextHandle);

	// 设置碰撞伤害覆盖值
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|AbilitySystem")
	static void SetCollisionDamageOverride(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, float Damage);

	// ========== 阵营辅助函数 ==========

	// 获取 Actor 的阵营标签
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|Faction")
	static FGameplayTag GetActorFaction(AActor* Actor);

	// 检查两个 Actor 是否为友方
	UFUNCTION(BlueprintPure, Category = "SailingHeart|Faction")
	static bool AreActorsFriends(AActor* FirstActor, AActor* SecondActor);

	// 检查两个 Actor 是否为敌方
	UFUNCTION(BlueprintPure, Category = "SailingHeart|Faction")
	static bool AreActorsEnemies(AActor* FirstActor, AActor* SecondActor);

	// ========== 伤害应用辅助函数 ==========

	/**
	 * 应用伤害（基于 Attack/Defence 计算，暴击在 ExecCalc 中判定）
	 * @param SourceASC 伤害来源的 ASC
	 * @param TargetASC 伤害目标的 ASC
	 * @param DamageEffectClass 伤害 GameplayEffect 类
	 */
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|Damage")
	static void ApplyDamage(
		UAbilitySystemComponent* SourceASC,
		UAbilitySystemComponent* TargetASC,
		TSubclassOf<UGameplayEffect> DamageEffectClass);

	/**
	 * 应用碰撞伤害（直接使用指定的伤害值，跳过 Attack/Defence 计算）
	 * @param SourceASC 伤害来源的 ASC
	 * @param TargetASC 伤害目标的 ASC
	 * @param DamageEffectClass 伤害 GameplayEffect 类
	 * @param DamageAmount 伤害值（通常是来源的剩余血量）
	 */
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|Damage")
	static void ApplyCollisionDamage(
		UAbilitySystemComponent* SourceASC,
		UAbilitySystemComponent* TargetASC,
		TSubclassOf<UGameplayEffect> DamageEffectClass,
		float DamageAmount);

	/**
	 * 应用 Buff/Debuff 效果
	 * @param SourceASC 效果来源的 ASC
	 * @param TargetASC 效果目标的 ASC
	 * @param EffectClass GameplayEffect 类
	 * @param Level 效果等级
	 */
	UFUNCTION(BlueprintCallable, Category = "SailingHeart|Effect")
	static void ApplyBuffDebuff(
		UAbilitySystemComponent* SourceASC,
		UAbilitySystemComponent* TargetASC,
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level = 1.f);
};
