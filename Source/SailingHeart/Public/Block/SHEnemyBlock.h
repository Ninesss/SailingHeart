// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Block/SHCombatBlockBase.h"
#include "AI/SHAIEntityInterface.h"
#include "SHEnemyBlock.generated.h"

class USHEnemyBlockData;

/**
 * 敌人方块 - 与玩家方块碰撞时互相造成伤害，通过 StateTree 主动攻击友军/玩家
 * 拥有 GAS 战斗能力
 */
UCLASS()
class SAILINGHEART_API ASHEnemyBlock : public ASHCombatBlockBase, public ISHAIEntityInterface
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
		int32 Level = 1,
		FRotator SpawnRotation = FRotator::ZeroRotator
	);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	FName GetEnemyTypeID() const { return BlockTypeID; }

	// ISHAIEntityInterface
	virtual const FSHAIConfig& GetAIConfig() const override;

protected:
	// 调试信息：红色显示敌人
	virtual FColor GetDebugColor() const override { return FColor::Red; }

	// 缓存的 DataAsset（SpawnDeferred 时写入）
	UPROPERTY()
	TObjectPtr<USHEnemyBlockData> EnemyBlockData;

	// 默认 AI 配置（EnemyBlockData 未设置时使用）
	FSHAIConfig DefaultAIConfig;

	friend class ASHEnemyBlock;
};
