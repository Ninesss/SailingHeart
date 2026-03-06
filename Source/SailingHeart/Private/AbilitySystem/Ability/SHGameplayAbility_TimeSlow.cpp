// Sailing Heart

#include "AbilitySystem/Ability/SHGameplayAbility_TimeSlow.h"
#include "Game/SHGameStateBase.h"
#include "Data/Ability/SHAbilityParams.h"
#include "Data/Ability/SHTimeSlowAbilityData.h"

USHGameplayAbility_TimeSlow::USHGameplayAbility_TimeSlow()
{
}

// ========== DataAsset 获取 ==========

USHTimeSlowAbilityData* USHGameplayAbility_TimeSlow::GetTimeSlowAbilityData() const
{
	return Cast<USHTimeSlowAbilityData>(GetAbilityData());
}

const FSHTimeSlowAbilityParams* USHGameplayAbility_TimeSlow::GetCurrentTimeSlowParams() const
{
	USHTimeSlowAbilityData* Data = GetTimeSlowAbilityData();
	if (Data)
	{
		return Data->GetParams(GetAbilityLevel());
	}
	return nullptr;
}

const FSHToggleAbilityParams* USHGameplayAbility_TimeSlow::GetToggleParams() const
{
	return GetCurrentTimeSlowParams();
}

void USHGameplayAbility_TimeSlow::OnToggleOn()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ASHGameStateBase* GameState = World->GetGameState<ASHGameStateBase>();
	if (GameState)
	{
		GameState->ActivateTimeSlow();
	}
}

void USHGameplayAbility_TimeSlow::OnToggleOff()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ASHGameStateBase* GameState = World->GetGameState<ASHGameStateBase>();
	if (GameState)
	{
		GameState->DeactivateTimeSlow();
	}
}
