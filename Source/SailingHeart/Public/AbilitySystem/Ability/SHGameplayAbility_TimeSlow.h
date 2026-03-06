// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/SHGameplayAbility_Toggle.h"
#include "SHGameplayAbility_TimeSlow.generated.h"

class ASHGameStateBase;
class USHTimeSlowAbilityData;
struct FSHTimeSlowAbilityParams;

/**
 * 时间减缓能力 - Toggle 模式
 * 按一下激活减速，再按一下关闭
 * 激活期间持续消耗能量（由基类处理），能量不足自动关闭
 *
 * 数据来源：USHTimeSlowAbilityData
 */
UCLASS()
class SAILINGHEART_API USHGameplayAbility_TimeSlow : public USHGameplayAbility_Toggle
{
	GENERATED_BODY()

public:
	USHGameplayAbility_TimeSlow();

protected:
	// ========== Toggle 接口实现 ==========

	virtual void OnToggleOn() override;
	virtual void OnToggleOff() override;

	// Toggle 参数获取（实现基类纯虚函数）
	virtual const FSHToggleAbilityParams* GetToggleParams() const override;

	// ========== DataAsset 获取 ==========

	USHTimeSlowAbilityData* GetTimeSlowAbilityData() const;
	const FSHTimeSlowAbilityParams* GetCurrentTimeSlowParams() const;
};
