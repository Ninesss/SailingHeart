// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "Data/Ability/SHAbilityParams.h"
#include "SHProjectileAbilityData.generated.h"

/**
 * 投射物能力数据
 * 用于配置投射物类型的能力
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHProjectileAbilityData : public USHAbilityDataBase
{
	GENERATED_BODY()

public:
	// 每级的参数配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Levels")
	TMap<int32, FSHProjectileAbilityParams> LevelParams;

	// ========== 辅助函数 ==========

	// 获取指定等级的参数
	const FSHProjectileAbilityParams* GetParams(int32 Level) const
	{
		if (const FSHProjectileAbilityParams* Params = LevelParams.Find(Level))
		{
			return Params;
		}
		// 回退到等级 1
		if (Level != 1)
		{
			return LevelParams.Find(1);
		}
		return nullptr;
	}

	virtual int32 GetMaxLevel() const override { return LevelParams.Num(); }
	virtual bool HasLevelConfig(int32 Level) const override { return LevelParams.Contains(Level); }

	virtual const FSHAbilityParams* GetBaseParams(int32 Level) const override
	{
		return GetParams(Level);
	}
};
