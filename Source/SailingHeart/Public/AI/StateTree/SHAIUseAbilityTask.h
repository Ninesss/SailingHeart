// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "SHAIUseAbilityTask.generated.h"

USTRUCT()
struct FSHAIUseAbilityTaskInstanceData
{
	GENERATED_BODY()

	// 绑定到 Evaluator 输出的 CurrentTarget
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// 当前轮到第几个技能（内部维护，从 0 开始）
	UPROPERTY()
	int32 CurrentAbilityIndex = 0;
};

/**
 * UseAbility Task - 通用技能触发
 * 通过 GameplayEvent 触发 ASC 上的技能，把目标塞进 EventData
 * 技能自行处理动画、Notify、攻击逻辑
 *
 * 多技能支持：
 * - AbilityEventTags 是有序的 Tag 列表，每次 EnterState 触发下一个
 * - 轮完一圈后从头开始
 *
 * 用法：在 Attack 子状态里放此 Task（瞬发），配合 Transition 实现循环触发
 * 冷却和能量消耗由 Ability 自己管理，Task 只负责发 Event
 */
USTRUCT(meta = (DisplayName = "SH AI Use Ability"))
struct SAILINGHEART_API FSHAIUseAbilityTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIUseAbilityTaskInstanceData;

	/**
	 * 按顺序轮流触发的技能 TriggerTag 列表，使用 Ability.Trigger.* 命名空间
	 * （如 Ability.Trigger.Projectile.Basic）
	 *
	 * 流程：
	 * 1. StateTree 进入 Attack 状态 → 本 Task 按 DynamicAbilityTags 找到对应 Spec
	 * 2. 调用 TriggerAbilityFromGameplayEvent 直接激活，Payload 携带目标信息
	 * 3. Ability 查找 AbilityMontageMap[TriggerTag] 拿到 Montage → 播放 → Notify 发送 CombatSocket.* 事件
	 * 4. Ability 监听 CombatSocket.* 事件 → 获取 Socket 位置 → 生成投射物
	 *
	 * 注意：这里填 Ability.Trigger.* 标签，不是 CombatSocket.* 标签
	 * 顺序 = 攻击顺序，触发完一轮后回到第一个
	 */
	UPROPERTY(EditAnywhere, Category = "AI")
	TArray<FGameplayTag> AbilityEventTags;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIUseAbilityTaskInstanceData::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
