// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SHGridFunctionLibrary.generated.h"

/**
 * 全局网格坐标系工具库
 * 用于敌人生成、场景块对齐等需要统一网格坐标的场景
 *
 * 前提：SHGridBase 的 Rows 和 Columns 都是偶数，这样全局网格自动对齐
 */
UCLASS()
class SAILINGHEART_API USHGridFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 网格坐标 → 世界坐标（返回格子中心位置）
	 */
	UFUNCTION(BlueprintPure, Category = "Grid")
	static FVector GridToWorld(int32 GridX, int32 GridY, float CellSize = 200.0f);

	/**
	 * 世界坐标 → 网格坐标
	 */
	UFUNCTION(BlueprintPure, Category = "Grid")
	static FIntPoint WorldToGrid(const FVector& WorldPos, float CellSize = 200.0f);

	/**
	 * 将世界坐标对齐到最近的网格中心
	 */
	UFUNCTION(BlueprintPure, Category = "Grid")
	static FVector SnapToGrid(const FVector& WorldPos, float CellSize = 200.0f);

	/**
	 * 场景块内的局部网格坐标 → 世界坐标
	 */
	UFUNCTION(BlueprintPure, Category = "Grid")
	static FVector ChunkGridToWorld(const FVector& ChunkOrigin, int32 LocalGridX, int32 LocalGridY, float CellSize = 200.0f);

	/**
	 * 世界坐标 → 场景块内的局部网格坐标
	 */
	UFUNCTION(BlueprintPure, Category = "Grid")
	static FIntPoint WorldToChunkGrid(const FVector& ChunkOrigin, const FVector& WorldPos, float CellSize = 200.0f);
};
