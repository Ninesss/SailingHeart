// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/SHGridBase.h"
#include "SHWorldBlockSpawner.generated.h"

class ASHGridBase;
class ASHGameStateBase;
class ASHEnemyBlock;
class ASHNeutralBlock;
class USHEnemyBlockData;
class USHNeutralBlockData;

/**
 * 世界方块生成器 - 同时支持生成敌人方块和中立方块
 */
UCLASS()
class SAILINGHEART_API ASHWorldBlockSpawner : public AActor
{
	GENERATED_BODY()

public:
	ASHWorldBlockSpawner();

	// ========== 敌人方块配置 ==========

	// 敌人方块数据列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Spawner")
	TArray<USHEnemyBlockData*> EnemyBlockDataList;

	// 敌人方块等级范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Spawner")
	int32 MinEnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Spawner")
	int32 MaxEnemyLevel = 1;

	// ========== 中立方块配置 ==========

	// 中立方块数据列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral Spawner")
	TArray<USHNeutralBlockData*> NeutralBlockDataList;

	// 中立方块等级范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral Spawner")
	int32 MinNeutralLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral Spawner")
	int32 MaxNeutralLevel = 2;

	// 中立方块生成权重（相对于敌人方块）
	// 例如：NeutralSpawnWeight = 0.3 表示 30% 概率生成中立方块
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neutral Spawner", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NeutralSpawnWeight = 0.3f;

	// ========== 通用配置 ==========

	// 生成方向（方块从哪个方向生成）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EGridMovementDirection SpawnDirection = EGridMovementDirection::PositiveX;

	// 生成间隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnInterval = 5.0f;

	// 在Grid前方多少个单元格生成（沿移动方向）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 SpawnDistanceOffset = 5;

	// 每次最少生成数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MinSpawnCount = 1;

	// 每次最多生成数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MaxSpawnCount = 3;

	// 生成位置的横向偏移范围（相对于Grid中心，单位：格）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MinLateralOffset = -8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MaxLateralOffset = 7;

	// BeginPlay时自动开始生成
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bAutoStart = true;

	// ========== 运行时控制 ==========

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	bool IsSpawning() const { return bIsSpawning; }

	// 手动生成一波方块
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnWave();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool bIsSpawning = false;
	FTimerHandle SpawnTimerHandle;

	UPROPERTY()
	ASHGridBase* CachedTargetGrid = nullptr;

	UPROPERTY()
	ASHGameStateBase* CachedGameState = nullptr;

	ASHGridBase* GetTargetGrid();

	// 生成单个方块（根据权重决定类型）
	void SpawnSingleBlock(const FVector& Location);

	// 生成敌人方块
	void SpawnEnemyBlock(const FVector& Location);

	// 生成中立方块
	void SpawnNeutralBlock(const FVector& Location);

	// 随机选择数据
	USHEnemyBlockData* GetRandomEnemyData();
	USHNeutralBlockData* GetRandomNeutralData();

	// 随机选择不重复的横向偏移
	TArray<int32> SelectRandomLateralOffsets(int32 Count);

	// 计算生成位置
	FVector CalculateSpawnLocation(int32 LateralOffset);
};
