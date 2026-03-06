// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Block/SHBlockBase.h"
#include "SHNeutralBlock.generated.h"

class ASHGridBase;
class USHNeutralBlockData;

/**
 * 中立方块 - 与玩家方块碰撞后转化为对应的玩家方块
 * 可以放置在世界中或由 Spawner 生成
 * 没有 GAS 战斗能力，只有碰撞转化逻辑
 */
UCLASS()
class SAILINGHEART_API ASHNeutralBlock : public ASHBlockBase
{
	GENERATED_BODY()

public:
	ASHNeutralBlock();

	// ========== 静态生成方法 ==========

	/**
	 * 使用 Deferred 方式生成中立方块（统一入口）
	 * @param World 世界
	 * @param NeutralData 中立方块 DataAsset
	 * @param Location 生成位置
	 * @param InLevel 方块等级
	 * @return 生成的方块，失败返回 nullptr
	 */
	static ASHNeutralBlock* SpawnDeferred(
		UWorld* World,
		USHNeutralBlockData* NeutralData,
		const FVector& Location,
		int32 InLevel
	);

	UFUNCTION(BlueprintCallable, Category = "Neutral")
	FName GetTargetBlockTypeID() const { return TargetBlockTypeID; }

	UFUNCTION(BlueprintCallable, Category = "Neutral")
	int32 GetNeutralLevel() const { return Level; }

protected:
	virtual void BeginPlay() override;
	virtual void DrawDebugInfo() override;

	// 对应的玩家方块类型 ID（如 "Cannon"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Neutral")
	FName TargetBlockTypeID;

	// 方块等级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Neutral")
	int32 Level = 1;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 碰撞处理
	UFUNCTION()
	void OnNeutralBlockBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 尝试在玩家 Grid 上生成玩家方块
	bool TrySpawnPlayerBlock(ASHGridBase* PlayerGrid);
};
