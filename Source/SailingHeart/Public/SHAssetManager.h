// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "SHAssetManager.generated.h"

/**
 * 自定义 AssetManager
 * 负责在游戏启动时初始化 Native GameplayTags
 */
UCLASS()
class SAILINGHEART_API USHAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static USHAssetManager& Get();

	/** 引擎启动时调用，初始化 GameplayTags */
	virtual void StartInitialLoading() override;
};
