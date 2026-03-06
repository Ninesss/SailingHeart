// Sailing Heart

#include "AbilitySystem/SHAbilitySystemGlobals.h"
#include "AbilitySystem/SHGameplayEffectContext.h"

FGameplayEffectContext* USHAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FSHGameplayEffectContext();
}
