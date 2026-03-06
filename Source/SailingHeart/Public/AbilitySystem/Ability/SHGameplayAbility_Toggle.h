// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/SHGameplayAbilityBase.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "SHGameplayAbility_Toggle.generated.h"

struct FSHToggleAbilityParams;

/**
 * Toggle 能力基类
 * 按一下激活，再按一下关闭
 * 提供统一的状态管理和冷却机制
 * 使用 LooseGameplayTag 管理状态
 *
 * 参数来源：DataAsset 中的 FSHToggleAbilityParams
 */
UCLASS(Abstract)
class SAILINGHEART_API USHGameplayAbility_Toggle : public USHGameplayAbilityBase
{
	GENERATED_BODY()

public:
	USHGameplayAbility_Toggle();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 能力被移除时清理状态（包括 ASC 销毁的情况）
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	// ========== 子类必须实现 ==========

	// 激活时调用
	virtual void OnToggleOn() PURE_VIRTUAL(USHGameplayAbility_Toggle::OnToggleOn, );

	// 关闭时调用
	virtual void OnToggleOff() PURE_VIRTUAL(USHGameplayAbility_Toggle::OnToggleOff, );

	// ========== 子类可选覆盖 ==========

	// 获取激活状态 Tag（默认从DataAsset读取）
	virtual FGameplayTag GetToggleActiveTag() const;

	// 激活前额外检查（默认检查能量是否足够）
	virtual bool CanToggleOn() const;

	// 关闭前额外检查（默认返回 true）
	virtual bool CanToggleOff() const { return true; }

	// 获取冷却 Tag（默认基于激活 Tag 生成）
	virtual FGameplayTag GetToggleCooldownTag() const;

	// ========== 参数获取（子类实现）==========

	// 获取 Toggle 参数（子类必须实现）
	virtual const FSHToggleAbilityParams* GetToggleParams() const PURE_VIRTUAL(USHGameplayAbility_Toggle::GetToggleParams, return nullptr;);

	// 获取当前冷却时间
	float GetCooldownDuration() const;

	// 获取每秒能量消耗
	float GetEnergyCostPerSecond() const;

	// ========== 状态查询 ==========

	// 是否处于激活状态
	UFUNCTION(BlueprintCallable, Category = "Toggle")
	bool IsToggleActive() const;

	// 是否在冷却中
	UFUNCTION(BlueprintCallable, Category = "Toggle")
	bool IsToggleOnCooldown() const;

	// ========== 工具函数 ==========

	// 应用冷却
	void ApplyToggleCooldown();

	// 移除激活状态（供子类在特殊情况下调用，如能量耗尽）
	void ForceDeactivateToggle();

private:
	// 内部激活逻辑
	void ActivateToggle();

	// 内部关闭逻辑
	void DeactivateToggle();

	// ========== 持续能量消耗 ==========

	// 能量消耗定时器
	FTimerHandle EnergyConsumeTimerHandle;

	// 能量消耗Tick间隔（秒）
	static constexpr float EnergyConsumeInterval = 0.1f;

	// 启动能量消耗
	void StartEnergyConsumption();

	// 停止能量消耗
	void StopEnergyConsumption();

	// 定时器回调：消耗能量
	void ConsumeEnergyTick();

	// 属性变化回调：检查能量是否不足
	void OnEnergyAttributeChanged(const FOnAttributeChangeData& Data);

	// 检查能量是否足够继续
	bool HasEnoughEnergyToContinue() const;
};
