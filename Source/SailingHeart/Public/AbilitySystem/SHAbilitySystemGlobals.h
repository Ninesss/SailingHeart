// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "SHAbilitySystemGlobals.generated.h"

/**
 * 自定义 AbilitySystemGlobals
 * 用于分配自定义的 GameplayEffectContext
 */
UCLASS()
class SAILINGHEART_API USHAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	// 覆盖分配器，返回自定义 Context
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
