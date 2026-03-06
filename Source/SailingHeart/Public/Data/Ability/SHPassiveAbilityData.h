// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SHPassiveAbilityData.generated.h"

class UGameplayEffect;
class UTexture2D;

/**
 * 被动能力数据资源
 * 被动能力使用 GameplayEffect 实现，永久生效
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHPassiveAbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ========== 基本信息 ==========

	// 能力 ID（唯一标识）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	FName PassiveID;

	// 显示名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	FText DisplayName;

	// 描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	FText Description;

	// 图标（UI 用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	TSoftObjectPtr<UTexture2D> Icon;

	// ========== 被动效果 ==========

	// 被动效果类（永久 GameplayEffect）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive|Effect")
	TSubclassOf<UGameplayEffect> PassiveEffectClass;

	// ========== 辅助函数 ==========

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PassiveAbility", GetFName());
	}
};
