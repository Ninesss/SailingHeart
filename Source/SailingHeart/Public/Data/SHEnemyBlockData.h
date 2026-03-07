// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Block/SHCombatBlockBase.h"
#include "SHEnemyBlockData.generated.h"

class ASHEnemyBlock;

/**
 * 敌人方块数据资源
 * 每种敌人类型一个 DataAsset 文件
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHEnemyBlockData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 敌人类型 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FName EnemyTypeID;

	// 敌人蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TSubclassOf<ASHEnemyBlock> EnemyClass;

	// 每级配置（Key = 等级，从 1 开始）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TMap<int32, FBlockLevelConfig> LevelConfigs;

	// 获取指定等级配置
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	FBlockLevelConfig GetLevelConfig(int32 Level) const;

	// 获取最大等级
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	int32 GetMaxLevel() const;

	// 检查等级是否有效
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool IsLevelValid(int32 Level) const { return LevelConfigs.Contains(Level); }
};
