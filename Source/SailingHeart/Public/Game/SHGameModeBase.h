// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SHGameModeBase.generated.h"

class USHPlayerCharacterData;

/**
 *
 */
UCLASS()
class SAILINGHEART_API ASHGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASHGameModeBase();

	// 重写 RestartPlayer 以使用选择的角色类
	virtual void RestartPlayer(AController* NewPlayer) override;

	// 获取玩家应使用的角色数据（优先使用玩家选择，fallback 到默认）
	UFUNCTION(BlueprintCallable, Category = "Character")
	USHPlayerCharacterData* GetCharacterDataForPlayer(AController* Controller) const;

	// 初始化玩家角色属性和能力
	void InitializePlayerCharacter(AController* Controller, USHPlayerCharacterData* CharacterData);

protected:
	// 测试用：直接选择角色 DataAsset（在蓝图中拖入即可）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	USHPlayerCharacterData* DefaultCharacterData;
};
