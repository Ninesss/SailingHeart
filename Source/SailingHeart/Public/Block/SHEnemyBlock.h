// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Block/SHCombatBlockBase.h"
#include "SHEnemyBlock.generated.h"

class USHEnemyBlockData;

/**
 * 敌人方块 - 与玩家方块碰撞时互相造成伤害
 * 拥有 GAS 战斗能力
 */
UCLASS()
class SAILINGHEART_API ASHEnemyBlock : public ASHCombatBlockBase
{
	GENERATED_BODY()

public:
	ASHEnemyBlock();

	// ========== 静态生成方法 ==========

	/**
	 * 使用 Deferred 方式生成敌人方块（统一入口）
	 * @param World 世界
	 * @param EnemyData 敌人 DataAsset
	 * @param Location 生成位置
	 * @param Level 敌人等级
	 * @return 生成的方块，失败返回 nullptr
	 */
	static ASHEnemyBlock* SpawnDeferred(
		UWorld* World,
		USHEnemyBlockData* EnemyData,
		const FVector& Location,
		int32 Level = 1
	);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	FName GetEnemyTypeID() const { return BlockTypeID; }

protected:
	// 调试信息：红色显示敌人
	virtual FColor GetDebugColor() const override { return FColor::Red; }
};
