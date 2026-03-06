// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "SHGameplayAbilityBase.generated.h"

class UAbilitySystemComponent;
class USHAbilityDataBase;
class USHAttributeSetBase;

/**
 * 能力基类 - 所有游戏能力的基类
 * 提供统一的默认配置和通用引用获取功能
 *
 * DataAsset 集成：
 * - 能力从 DataAsset 读取配置参数
 * - DataAsset 通过 Spec.SourceObject 传入
 * - 子类使用具体的 DataAsset 类型（如 USHProjectileAbilityData）
 */
UCLASS(Abstract)
class SAILINGHEART_API USHGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USHGameplayAbilityBase();

	// ========== 常用引用获取 ==========

	// 获取拥有者的 ASC
	UFUNCTION(BlueprintCallable, Category = "Ability|Reference")
	UAbilitySystemComponent* GetOwnerASC() const;

	// 获取源 Actor（OwnerActor，通常是 PlayerState 或 Block）
	UFUNCTION(BlueprintCallable, Category = "Ability|Reference")
	AActor* GetSourceActor() const;

	// 获取头像 Actor（AvatarActor，执行者，通常是角色或方块）
	UFUNCTION(BlueprintCallable, Category = "Ability|Reference")
	AActor* GetAvatarActor() const;

	// 获取拥有者的 Controller（如果是 Pawn）
	UFUNCTION(BlueprintCallable, Category = "Ability|Reference")
	AController* GetOwnerController() const;

	// ========== DataAsset 支持 ==========

	// 获取能力数据资产基类（从 Spec.SourceObject 获取）
	// 授予能力时必须通过 SourceObject 传入 DataAsset
	// 子类应该转换为具体类型使用
	UFUNCTION(BlueprintCallable, Category = "Ability|Data")
	USHAbilityDataBase* GetAbilityData() const;

protected:
	// ========== Cost系统 ==========

	// 检查能量是否足够
	UFUNCTION(BlueprintCallable, Category = "Ability|Cost")
	bool HasEnoughEnergy(float Amount) const;

	// 消耗能量（Server端执行）
	UFUNCTION(BlueprintCallable, Category = "Ability|Cost")
	void SpendEnergy(float Amount);

	// 获取当前能量
	UFUNCTION(BlueprintCallable, Category = "Ability|Cost")
	float GetCurrentEnergy() const;

	// 获取一次性Cost（子类重写）
	virtual float GetEnergyCost() const;

	// ========== Cooldown系统 ==========

	// 检查是否在冷却中
	UFUNCTION(BlueprintCallable, Category = "Ability|Cooldown")
	bool IsAbilityOnCooldown() const;

	// 应用冷却（使用GameplayTag + Timer）
	void ApplyAbilityCooldown();

	// 获取冷却Tag（基于AbilityTags生成，子类可重写）
	virtual FGameplayTag GetAbilityCooldownTag() const;

	// 获取冷却时间（子类重写）
	virtual float GetAbilityCooldownDuration() const;

	// ========== 统一检查 ==========

	// 检查Cost和Cooldown，成功则消耗能量并应用冷却
	// 用于替代CommitAbility，供一次性技能使用
	UFUNCTION(BlueprintCallable, Category = "Ability|Cost")
	bool CheckAndCommitAbilityCost();

private:
	// 获取基础 AttributeSet
	const USHAttributeSetBase* GetBaseAttributeSet() const;
};
