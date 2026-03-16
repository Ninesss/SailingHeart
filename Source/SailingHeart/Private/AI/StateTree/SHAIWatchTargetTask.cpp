// Sailing Heart

#include "AI/StateTree/SHAIWatchTargetTask.h"
#include "Block/SHBlockBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

EStateTreeRunStatus FSHAIWatchTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSHAIWatchTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FSHAIWatchTargetTaskInstanceData& InstanceData = Context.GetInstanceData(*this);

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Running;
	}

	// 只旋转 FunctionalSKM（方块类才有）
	ASHBlockBase* Block = Cast<ASHBlockBase>(OwnerActor);
	if (!Block)
	{
		return EStateTreeRunStatus::Running;
	}

	USkeletalMeshComponent* SKM = Block->GetFunctionalSKM();
	if (!SKM)
	{
		return EStateTreeRunStatus::Running;
	}

	// SKM 正在模拟物理（Ragdoll）时跳过旋转
	if (SKM->IsSimulatingPhysics())
	{
		return EStateTreeRunStatus::Running;
	}

	// SKM RelativeRotation = WorldFacingYaw - ActorYaw - 90
	// 减去 ActorYaw 是因为 RelativeRotation 是相对于 Actor 的，需要转换到局部空间
	// -90 是 mesh 资产朝向的固定偏移
	const float ActorYaw = OwnerActor->GetActorRotation().Yaw;

	FRotator TargetRotation;
	if (InstanceData.CurrentTarget)
	{
		const FVector ToTarget = InstanceData.CurrentTarget->GetActorLocation() - OwnerActor->GetActorLocation();
		TargetRotation = FRotator(0.f, ToTarget.Rotation().Yaw - ActorYaw - 90.f, 0.f);
	}
	else
	{
		// 无目标：默认朝向 = Actor forward 方向，转换到局部空间后恒为 -90
		TargetRotation = FRotator(0.f, -90.f, 0.f);
	}

	if (TurnSpeed <= 0.f || DeltaTime <= 0.f)
	{
		SKM->SetRelativeRotation(TargetRotation);
	}
	else
	{
		const FRotator CurrentRotation = SKM->GetRelativeRotation();
		const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurnSpeed / 360.f);
		SKM->SetRelativeRotation(NewRotation);
	}

	return EStateTreeRunStatus::Running;
}

void FSHAIWatchTargetTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 退出时重置 SKM 朝向（可选，根据动画需求决定是否要 Reset）
}
