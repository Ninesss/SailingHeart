// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Block/SHCombatBlockBase.h"
#include "SHPlayerBlockData.generated.h"

class ASHPlayerBlock;
class UStateTree;

/**
 * 玩家方块数据资源
 * 每种方块类型一个 DataAsset 文件
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHPlayerBlockData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 方块类型 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	FName BlockTypeID;

	// 方块蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	TSubclassOf<ASHPlayerBlock> BlockClass;

	// StateTree 行为树（可选，用于 AI 行为控制）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UStateTree> StateTree;

	// 每级配置（Key = 等级，从 1 开始）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TMap<int32, FBlockLevelConfig> LevelConfigs;

	// 获取指定等级配置
	UFUNCTION(BlueprintCallable, Category = "Block")
	FBlockLevelConfig GetLevelConfig(int32 Level) const;

	// 获取最大等级
	UFUNCTION(BlueprintCallable, Category = "Block")
	int32 GetMaxLevel() const;

	// 检查等级是否有效
	UFUNCTION(BlueprintCallable, Category = "Block")
	bool IsLevelValid(int32 Level) const { return LevelConfigs.Contains(Level); }
};
