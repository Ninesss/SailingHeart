// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "Data/Ability/SHAbilityParams.h"
#include "SHTimeSlowAbilityData.generated.h"

/**
 * 时间减缓能力数据
 * 用于配置时间减缓类型的能力
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHTimeSlowAbilityData : public USHAbilityDataBase
{
	GENERATED_BODY()

public:
	// 每级的参数配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Levels")
	TMap<int32, FSHTimeSlowAbilityParams> LevelParams;

	// ========== 辅助函数 ==========

	// 获取指定等级的参数
	const FSHTimeSlowAbilityParams* GetParams(int32 Level) const
	{
		if (const FSHTimeSlowAbilityParams* Params = LevelParams.Find(Level))
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
