// Sailing Heart

#include "SHAssetManager.h"
#include "SHGameplayTags.h"

USHAssetManager& USHAssetManager::Get()
{
	check(GEngine);

	USHAssetManager* AssetManager = Cast<USHAssetManager>(GEngine->AssetManager);
	return *AssetManager;
}

void USHAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// 初始化 Native GameplayTags
	FSHGameplayTags::InitializeNativeGameplayTags();
}
