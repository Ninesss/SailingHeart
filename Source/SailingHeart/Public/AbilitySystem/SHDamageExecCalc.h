// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "SHDamageExecCalc.generated.h"

/**
 * 伤害执行计算
 * 支持两种伤害类型：
 * - 普通伤害：基于 Attack 和 Defence 计算
 * - 碰撞伤害：直接使用 Context 中的 CollisionDamageOverride
 */
UCLASS()
class SAILINGHEART_API USHDamageExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	USHDamageExecCalc();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
