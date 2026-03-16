// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "SHAIWatchTargetTask.generated.h"

USTRUCT()
struct FSHAIWatchTargetTaskInstanceData
{
	GENERATED_BODY()

	// 绑定到 Evaluator 输出的 CurrentTarget
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;
};

/**
 * WatchTarget Task（不可移动单位专用）
 * 每 Tick 将 FunctionalSKM 的 Root 旋转朝向 CurrentTarget
 * CurrentTarget 为空时慢慢转回 DefaultRotation（用于 Idle 状态回正）
 * 供 ASHEnemyBlock、ASHPlayerBlock 在 Alert/Idle 状态下使用
 *
 * 注意：此 Task 永远返回 Running，状态切换由 Transition Condition 控制
 */
USTRUCT(meta = (DisplayName = "SH AI Watch Target"))
struct SAILINGHEART_API FSHAIWatchTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIWatchTargetTaskInstanceData;

	// 朝向插值速度（Yaw 转向 deg/s，0 = 立即）
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	float TurnSpeed = 360.f;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIWatchTargetTaskInstanceData::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
