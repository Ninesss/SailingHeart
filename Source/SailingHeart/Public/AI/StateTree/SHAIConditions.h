// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeExecutionContext.h"
#include "AI/StateTree/SHAIEvaluator.h"
#include "SHAIConditions.generated.h"

// ============================================================
//  HasTarget Condition Instance Data
// ============================================================

USTRUCT()
struct FSHAIHasTargetConditionInstanceData
{
	GENERATED_BODY()

	// 绑定到 Evaluator 输出的 CurrentTarget
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;
};

/**
 * 条件：当前是否有有效目标
 * 绑定 Evaluator 的 CurrentTarget，不为空且未死亡时为 true
 */
USTRUCT(meta = (DisplayName = "SH AI Has Target"))
struct SAILINGHEART_API FSHAIHasTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIHasTargetConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIHasTargetConditionInstanceData::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================
//  InDetectionRange Condition Instance Data
// ============================================================

USTRUCT()
struct FSHAIInDetectionRangeConditionInstanceData
{
	GENERATED_BODY()

	// 绑定到 Evaluator 输出的 bInDetectionRange
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bInDetectionRange = false;
};

/**
 * 条件：目标是否在检测范围内
 * 绑定 Evaluator 的 bInDetectionRange
 */
USTRUCT(meta = (DisplayName = "SH AI In Detection Range"))
struct SAILINGHEART_API FSHAIInDetectionRangeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIInDetectionRangeConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIInDetectionRangeConditionInstanceData::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================
//  InAttackRange Condition Instance Data
// ============================================================

USTRUCT()
struct FSHAIInAttackRangeConditionInstanceData
{
	GENERATED_BODY()

	// 绑定到 Evaluator 输出的 bInAttackRange
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bInAttackRange = false;
};

/**
 * 条件：目标是否在攻击范围内
 * 绑定 Evaluator 的 bInAttackRange
 */
USTRUCT(meta = (DisplayName = "SH AI In Attack Range"))
struct SAILINGHEART_API FSHAIInAttackRangeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIInAttackRangeConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIInAttackRangeConditionInstanceData::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================
//  IsDead Condition（无需绑定，直接读取 Owner 的战斗接口）
// ============================================================

USTRUCT()
struct FSHAIIsDeadConditionInstanceData
{
	GENERATED_BODY()
};

/**
 * 条件：Owner 是否已死亡
 * 直接调用 ISHCombatInterface::IsDead()，无需 InstanceData 绑定
 * 用于 StateTree 根层级 Global Transition → Dead 状态
 */
USTRUCT(meta = (DisplayName = "SH AI Is Dead"))
struct SAILINGHEART_API FSHAIIsDeadCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIIsDeadConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIIsDeadConditionInstanceData::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
