// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SHNeutralBlockData.generated.h"

class ASHNeutralBlock;

/**
 * 中立方块数据资源
 * 每种中立方块类型一个 DataAsset 文件
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHNeutralBlockData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 中立方块类型 ID（如 "Cannon_Inactive"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral")
	FName NeutralTypeID;

	// 对应的玩家方块类型 ID（如 "Cannon"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral")
	FName TargetBlockTypeID;

	// 中立方块蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral")
	TSubclassOf<ASHNeutralBlock> NeutralClass;
};
