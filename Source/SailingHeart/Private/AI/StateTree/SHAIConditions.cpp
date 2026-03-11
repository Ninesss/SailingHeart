// Sailing Heart

#include "AI/StateTree/SHAIConditions.h"
#include "Interface/SHCombatInterface.h"

// ============================================================
//  FSHAIHasTargetCondition
// ============================================================

bool FSHAIHasTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FSHAIHasTargetConditionInstanceData& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.CurrentTarget)
	{
		return false;
	}

	// 过滤已死亡目标
	if (InstanceData.CurrentTarget->Implements<USHCombatInterface>())
	{
		if (ISHCombatInterface::Execute_IsDead(InstanceData.CurrentTarget))
		{
			return false;
		}
	}

	return true;
}

// ============================================================
//  FSHAIInDetectionRangeCondition
// ============================================================

bool FSHAIInDetectionRangeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FSHAIInDetectionRangeConditionInstanceData& InstanceData = Context.GetInstanceData(*this);
	return InstanceData.bInDetectionRange;
}

// ============================================================
//  FSHAIInAttackRangeCondition
// ============================================================

bool FSHAIInAttackRangeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FSHAIInAttackRangeConditionInstanceData& InstanceData = Context.GetInstanceData(*this);
	return InstanceData.bInAttackRange;
}
