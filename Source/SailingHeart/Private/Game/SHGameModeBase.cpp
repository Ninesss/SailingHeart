// Sailing Heart

#include "Game/SHGameModeBase.h"
#include "Game/SHGameStateBase.h"
#include "Player/SHPlayerState.h"
#include "Player/SHPlayerController.h"
#include "Data/SHPlayerCharacterData.h"
#include "Character/SHPlayerCharacterBase.h"

ASHGameModeBase::ASHGameModeBase()
{
	// 设置默认类
	GameStateClass = ASHGameStateBase::StaticClass();
	PlayerStateClass = ASHPlayerState::StaticClass();
	PlayerControllerClass = ASHPlayerController::StaticClass();
}

void ASHGameModeBase::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	// 获取角色数据
	USHPlayerCharacterData* CharacterData = GetCharacterDataForPlayer(NewPlayer);

	if (CharacterData && CharacterData->CharacterClass)
	{
		// 临时修改 DefaultPawnClass
		TSubclassOf<APawn> OriginalPawnClass = DefaultPawnClass;
		DefaultPawnClass = CharacterData->CharacterClass;

		// 调用父类生成角色
		Super::RestartPlayer(NewPlayer);

		// 恢复原始 PawnClass
		DefaultPawnClass = OriginalPawnClass;

		// 初始化角色属性和能力
		InitializePlayerCharacter(NewPlayer, CharacterData);
	}
	else
	{
		// 没有角色数据，使用默认流程
		Super::RestartPlayer(NewPlayer);
	}
}

USHPlayerCharacterData* ASHGameModeBase::GetCharacterDataForPlayer(AController* Controller) const
{
	if (!Controller)
	{
		return DefaultCharacterData;
	}

	// 优先使用玩家选择的角色
	ASHPlayerState* PS = Controller->GetPlayerState<ASHPlayerState>();
	if (PS && !PS->GetSelectedCharacterID().IsNone())
	{
		ASHGameStateBase* GS = GetGameState<ASHGameStateBase>();
		if (GS)
		{
			USHPlayerCharacterData* SelectedData = GS->GetPlayerCharacterData(PS->GetSelectedCharacterID());
			if (SelectedData)
			{
				return SelectedData;
			}
		}
	}

	// Fallback 到默认角色
	return DefaultCharacterData;
}

void ASHGameModeBase::InitializePlayerCharacter(AController* Controller, USHPlayerCharacterData* CharacterData)
{
	if (!Controller || !CharacterData)
	{
		return;
	}

	// 获取 PlayerState 并初始化属性
	ASHPlayerState* PS = Controller->GetPlayerState<ASHPlayerState>();
	if (PS)
	{
		PS->InitializeFromCharacterData(CharacterData);
	}
}




