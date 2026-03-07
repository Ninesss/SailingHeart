// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Block/SHCombatBlockBase.h"
#include "Save/SHBlockTypes.h"
#include "SHPlayerBlock.generated.h"

class UGameplayEffect;
class ASHGridBase;

/**
 * 玩家方块 - 可被玩家操作（搬运、合成）的方块
 * 放置在玩家 Grid 上，拥有 GAS 战斗能力
 */
UCLASS()
class SAILINGHEART_API ASHPlayerBlock : public ASHCombatBlockBase
{
	GENERATED_BODY()

public:
	ASHPlayerBlock();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========== 静态生成方法 ==========

	/**
	 * 使用 Deferred 方式生成玩家方块（统一入口）
	 * @param World 世界
	 * @param BlockClass 方块蓝图类
	 * @param Grid 目标 Grid
	 * @param Row 行
	 * @param Column 列
	 * @param BlockTypeID 方块类型 ID
	 * @param Level 等级
	 * @param LevelConfig 等级配置
	 * @param CurrentHealth 当前血量（-1 表示满血）
	 * @return 生成的方块，失败返回 nullptr
	 */
	static ASHPlayerBlock* SpawnDeferred(
		UWorld* World,
		TSubclassOf<ASHPlayerBlock> BlockClass,
		ASHGridBase* Grid,
		int32 Row, int32 Column,
		FName BlockTypeID,
		int32 Level,
		const FBlockLevelConfig& LevelConfig,
		float CurrentHealth = -1.f
	);

	// ========== 升级 ==========

	// 直接升级到指定等级（技能/道具调用）
	// bRestoreFullHealth: true 回满血，false 保持当前血量比例
	UFUNCTION(BlueprintCallable, Category = "Block")
	bool UpgradeToLevel(int32 NewLevel, const FBlockLevelConfig& NewLevelConfig, bool bRestoreFullHealth = true);

	// ========== 合成 ==========

	// 合成另一个方块（调用者保留，等级+1，回满血）
	// 返回 true 表示合成成功
	UFUNCTION(BlueprintCallable, Category = "Block")
	bool MergeWith(const FBlockCarryState& OtherBlockState, const FBlockLevelConfig& NewLevelConfig);

	// ========== 搬运 ==========

	// 创建搬运状态
	UFUNCTION(BlueprintCallable, Category = "Block")
	FBlockCarryState CreateCarryState() const;

	// ========== Grid 位置 ==========

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetCell(int32 Row, int32 Column);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetOwnerGrid(ASHGridBase* Grid);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FGridCell GetCell() const { return Cell; }

	UFUNCTION(BlueprintCallable, Category = "Grid")
	ASHGridBase* GetOwnerGrid() const { return OwnerGrid.Get(); }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 调试信息：显示 Cell 位置
	virtual FString GetExtraDebugText() const override;
	virtual FColor GetDebugColor() const override { return FColor::Yellow; }

	// ========== 碰撞处理 ==========

	// 碰撞伤害 GameplayEffect（需要在蓝图中设置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> CollisionDamageEffect;

	UFUNCTION()
	void OnBlockCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 覆盖死亡处理（清理 Grid 数据）
	virtual void HandleDeath_Implementation() override;

	// 已处理过碰撞的敌人方块（避免重复触发，使用弱引用避免内存泄漏）
	TSet<TWeakObjectPtr<AActor>> ProcessedCollisions;

	// ========== 复制属性 ==========

	// 所在单元格
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Grid")
	FGridCell Cell;

	// 所属 Grid（弱引用，不复制）
	UPROPERTY()
	TWeakObjectPtr<ASHGridBase> OwnerGrid;

};
