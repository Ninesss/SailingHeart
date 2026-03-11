// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "SHAIMoveToTargetTask.generated.h"

USTRUCT()
struct FSHAIMoveToTargetTaskInstanceData
{
	GENERATED_BODY()

	// 绑定到 Evaluator 输出的 CurrentTarget
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// 绑定到 Evaluator 输出的 bInAttackRange
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bInAttackRange = false;
};

/**
 * MoveToTarget Task（可移动角色专用）
 * 每 Tick 通过 AddMovementInput 朝 CurrentTarget 移动
 * 进入攻击范围时返回 Succeeded，目标消失返回 Failed
 * 供 ASHEnemyCharacterBase、（未来）ASHAllyCharacterBase 在 Chase 状态下使用
 */
USTRUCT(meta = (DisplayName = "SH AI Move To Target"))
struct SAILINGHEART_API FSHAIMoveToTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIMoveToTargetTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIMoveToTargetTaskInstanceData::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
