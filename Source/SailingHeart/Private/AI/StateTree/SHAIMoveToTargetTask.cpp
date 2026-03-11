// Sailing Heart

#include "AI/StateTree/SHAIMoveToTargetTask.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/SHCombatInterface.h"

EStateTreeRunStatus FSHAIMoveToTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FSHAIMoveToTargetTaskInstanceData& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.CurrentTarget)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 进入追击时如果已在攻击范围内直接 Succeed（切换到 Attack 状态）
	if (InstanceData.bInAttackRange)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSHAIMoveToTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FSHAIMoveToTargetTaskInstanceData& InstanceData = Context.GetInstanceData(*this);

	// 目标消失或已死亡
	if (!InstanceData.CurrentTarget)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.CurrentTarget->Implements<USHCombatInterface>())
	{
		if (ISHCombatInterface::Execute_IsDead(InstanceData.CurrentTarget))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	// 已进入攻击范围，通知 StateTree 切换到 Attack 状态
	if (InstanceData.bInAttackRange)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 获取角色
	ACharacter* Character = Cast<ACharacter>(Context.GetOwner());
	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}

	// AddMovementInput 朝目标方向（XY 平面，忽略 Z 轴差异）
	FVector Direction = InstanceData.CurrentTarget->GetActorLocation() - Character->GetActorLocation();
	Direction.Z = 0.f;
	const FVector SafeDir = Direction.GetSafeNormal();

	if (!SafeDir.IsNearlyZero())
	{
		Character->AddMovementInput(SafeDir, 1.f);
	}

	return EStateTreeRunStatus::Running;
}

void FSHAIMoveToTargetTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 停止移动（清空输入）
	ACharacter* Character = Cast<ACharacter>(Context.GetOwner());
	if (Character)
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
	}
}
