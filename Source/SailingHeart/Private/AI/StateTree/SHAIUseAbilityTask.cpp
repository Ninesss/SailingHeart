// Sailing Heart

#include "AI/StateTree/SHAIUseAbilityTask.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Interface/SHCombatInterface.h"

EStateTreeRunStatus FSHAIUseAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (AbilityEventTags.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	FSHAIUseAbilityTaskInstanceData& InstanceData = Context.GetInstanceData(*this);
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());

	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 验证目标有效性
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

	// 取当前轮到的 EventTag（循环）
	const int32 TagCount = AbilityEventTags.Num();
	const int32 Index = InstanceData.CurrentAbilityIndex % TagCount;
	const FGameplayTag& EventTag = AbilityEventTags[Index];

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor);
	UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 通过 DynamicAbilityTags 找到对应的 AbilitySpec
	FGameplayAbilitySpec* Spec = nullptr;
	for (FGameplayAbilitySpec& S : ASC->GetActivatableAbilities())
	{
		if (S.GetDynamicSpecSourceTags().HasTagExact(EventTag))
		{
			Spec = &S;
			break;
		}
	}
	if (!Spec)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 构造 Payload，Target = 当前攻击目标（携带目标信息给 Ability 使用）
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = InstanceData.CurrentTarget;

	// 直接触发，无需 AbilityTriggers
	ASC->TriggerAbilityFromGameplayEvent(Spec->Handle, ASC->AbilityActorInfo.Get(), EventTag, &Payload, *ASC);

	// 推进到下一个技能（下次 EnterState 时使用）
	InstanceData.CurrentAbilityIndex = (Index + 1) % TagCount;

	// Task 瞬发完成，StateTree 继续执行（切回等待状态由外层 Transition 控制）
	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FSHAIUseAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// 瞬发 Task 不需要 Tick 逻辑
	return EStateTreeRunStatus::Succeeded;
}

void FSHAIUseAbilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 无需清理
}
