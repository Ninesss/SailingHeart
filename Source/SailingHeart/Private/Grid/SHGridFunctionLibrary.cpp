// Sailing Heart

#include "Grid/SHGridFunctionLibrary.h"

FVector USHGridFunctionLibrary::GridToWorld(int32 GridX, int32 GridY, float CellSize)
{
	return FVector(
		(GridX + 0.5f) * CellSize,
		(GridY + 0.5f) * CellSize,
		0.0f
	);
}

FIntPoint USHGridFunctionLibrary::WorldToGrid(const FVector& WorldPos, float CellSize)
{
	return FIntPoint(
		FMath::FloorToInt(WorldPos.X / CellSize),
		FMath::FloorToInt(WorldPos.Y / CellSize)
	);
}

FVector USHGridFunctionLibrary::SnapToGrid(const FVector& WorldPos, float CellSize)
{
	const FIntPoint GridCoord = WorldToGrid(WorldPos, CellSize);
	return GridToWorld(GridCoord.X, GridCoord.Y, CellSize);
}

FVector USHGridFunctionLibrary::ChunkGridToWorld(const FVector& ChunkOrigin, int32 LocalGridX, int32 LocalGridY, float CellSize)
{
	return ChunkOrigin + FVector(
		(LocalGridX + 0.5f) * CellSize,
		(LocalGridY + 0.5f) * CellSize,
		0.0f
	);
}

FIntPoint USHGridFunctionLibrary::WorldToChunkGrid(const FVector& ChunkOrigin, const FVector& WorldPos, float CellSize)
{
	const FVector LocalPos = WorldPos - ChunkOrigin;
	return FIntPoint(
		FMath::FloorToInt(LocalPos.X / CellSize),
		FMath::FloorToInt(LocalPos.Y / CellSize)
	);
}
