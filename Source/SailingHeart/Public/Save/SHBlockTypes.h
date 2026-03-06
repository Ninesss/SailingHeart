// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "SHBlockTypes.generated.h"

// ========== Grid 单元格位置 ==========
USTRUCT(BlueprintType)
struct FGridCell
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Grid")
	int32 Row = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Grid")
	int32 Column = 0;

	FGridCell() = default;
	FGridCell(int32 InRow, int32 InColumn) : Row(InRow), Column(InColumn) {}

	bool operator==(const FGridCell& Other) const
	{
		return Row == Other.Row && Column == Other.Column;
	}

	bool operator!=(const FGridCell& Other) const
	{
		return !(*this == Other);
	}

	// 用于 TMap 的 hash
	friend uint32 GetTypeHash(const FGridCell& Cell)
	{
		return HashCombine(GetTypeHash(Cell.Row), GetTypeHash(Cell.Column));
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("(%d, %d)"), Row, Column);
	}
};

// ========== 搬运状态（简化版）==========
USTRUCT(BlueprintType)
struct FBlockCarryState
{
	GENERATED_BODY()

	// 方块类型 ID
	UPROPERTY(BlueprintReadWrite)
	FName BlockTypeID;

	// 当前等级
	UPROPERTY(BlueprintReadWrite)
	int32 Level = 1;

	// 当前血量
	UPROPERTY(BlueprintReadWrite)
	float CurrentHealth = 100.f;

	FBlockCarryState() = default;
	FBlockCarryState(FName InBlockTypeID, int32 InLevel, float InHealth)
		: BlockTypeID(InBlockTypeID), Level(InLevel), CurrentHealth(InHealth) {}
};
