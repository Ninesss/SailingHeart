// Sailing Heart

#include "AI/StateTree/SHAIEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "AI/SHAIEntityInterface.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "Interface/SHCombatInterface.h"
#include "Engine/OverlapResult.h"

void FSHAIEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FSHAIEvaluatorInstanceData& InstanceData = Context.GetInstanceData(*this);
	InstanceData.CurrentTarget = nullptr;
	InstanceData.DistanceToTarget = 0.f;
	InstanceData.bInDetectionRange = false;
	InstanceData.bInAttackRange = false;

	// 启动时立即做一次检测
	UpdateTarget(Context);
}

void FSHAIEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UpdateTarget(Context);
}

void FSHAIEvaluator::UpdateTarget(FStateTreeExecutionContext& Context) const
{
	FSHAIEvaluatorInstanceData& InstanceData = Context.GetInstanceData(*this);

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return;
	}

	ISHAIEntityInterface* AIEntity = Cast<ISHAIEntityInterface>(OwnerActor);
	if (!AIEntity)
	{
		return;
	}
	const FSHAIConfig& Config = AIEntity->GetAIConfig();

	// 做球形检测（DetectionRange 内的所有动态 Actor）
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams(FCollisionObjectQueryParams::AllDynamicObjects);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Config.DetectionRange);

	const FVector Origin = OwnerActor->GetActorLocation();
	OwnerActor->GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		ObjectParams,
		SphereShape
	);

	// 筛选敌对且存活的 Actor，取最近的
	AActor* NearestTarget = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == OwnerActor)
		{
			continue;
		}

		if (!USHAbilitySystemLibrary::AreActorsEnemies(OwnerActor, Candidate))
		{
			continue;
		}

		if (Candidate->Implements<USHCombatInterface>())
		{
			if (ISHCombatInterface::Execute_IsDead(Candidate))
			{
				continue;
			}
		}

		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestTarget = Candidate;
		}
	}

	if (NearestTarget)
	{
		const float Distance = FMath::Sqrt(NearestDistSq);
		InstanceData.CurrentTarget = NearestTarget;
		InstanceData.DistanceToTarget = Distance;
		InstanceData.bInDetectionRange = (Distance <= Config.DetectionRange);
		InstanceData.bInAttackRange = (Distance <= Config.AttackRange);
	}
	else
	{
		InstanceData.CurrentTarget = nullptr;
		InstanceData.DistanceToTarget = 0.f;
		InstanceData.bInDetectionRange = false;
		InstanceData.bInAttackRange = false;
	}
}
