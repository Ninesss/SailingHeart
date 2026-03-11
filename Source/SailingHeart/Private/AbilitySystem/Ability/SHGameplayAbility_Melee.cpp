// Sailing Heart

#include "AbilitySystem/Ability/SHGameplayAbility_Melee.h"
#include "AbilitySystemComponent.h"

USHGameplayAbility_Melee::USHGameplayAbility_Melee()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USHGameplayAbility_Melee::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// TODO: 近战逻辑待实现
	// 预期流程：
	// 1. 缓存目标（从 TriggerEventData 或自动寻敌）
	// 2. 播放攻击 Montage（通过 ISHCombatInterface::GetAbilityMontage(TriggerTag) 从 Avatar 获取）
	// 3. 注册 WaitGameplayEvent 等待 AnimNotify
	// 4. Notify 触发后：对目标施加 DamageGameplayEffect

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UAbilitySystemComponent* USHGameplayAbility_Melee::GetDamageSourceASC_Implementation() const
{
	return GetOwnerASC();
}

AActor* USHGameplayAbility_Melee::GetDamageSourceActor_Implementation() const
{
	return GetSourceActor();
}
