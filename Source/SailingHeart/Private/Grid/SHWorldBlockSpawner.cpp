// Sailing Heart

#include "Grid/SHWorldBlockSpawner.h"
#include "Grid/SHGridBase.h"
#include "Block/SHEnemyBlock.h"
#include "Block/SHNeutralBlock.h"
#include "Data/SHEnemyBlockData.h"
#include "Data/SHNeutralBlockData.h"
#include "Game/SHGameStateBase.h"
#include "TimerManager.h"

ASHWorldBlockSpawner::ASHWorldBlockSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ASHWorldBlockSpawner::BeginPlay()
{
	Super::BeginPlay();

	CachedGameState = GetWorld()->GetGameState<ASHGameStateBase>();

	if (HasAuthority() && bAutoStart)
	{
		StartSpawning();
	}
}

void ASHWorldBlockSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void ASHWorldBlockSpawner::StartSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	ASHGridBase* TargetGrid = GetTargetGrid();
	if (!TargetGrid)
	{
		return;
	}

	// 检查是否至少有一种方块可生成
	if (EnemyBlockDataList.Num() == 0 && NeutralBlockDataList.Num() == 0)
	{
		return;
	}

	bIsSpawning = true;

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ASHWorldBlockSpawner::SpawnWave,
		SpawnInterval,
		true
	);
}

void ASHWorldBlockSpawner::StopSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsSpawning = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void ASHWorldBlockSpawner::SpawnWave()
{
	if (!HasAuthority())
	{
		return;
	}

	ASHGridBase* TargetGrid = GetTargetGrid();
	if (!TargetGrid)
	{
		return;
	}

	const int32 SpawnCount = FMath::RandRange(MinSpawnCount, MaxSpawnCount);
	TArray<int32> SelectedOffsets = SelectRandomLateralOffsets(SpawnCount);

	for (int32 Offset : SelectedOffsets)
	{
		FVector SpawnLocation = CalculateSpawnLocation(Offset);
		SpawnSingleBlock(SpawnLocation);
	}
}

void ASHWorldBlockSpawner::SpawnSingleBlock(const FVector& Location)
{
	const bool bHasEnemy = EnemyBlockDataList.Num() > 0;
	const bool bHasNeutral = NeutralBlockDataList.Num() > 0;

	if (!bHasEnemy && !bHasNeutral)
	{
		return;
	}

	// 根据权重决定生成类型
	bool bSpawnNeutral = false;

	if (bHasEnemy && bHasNeutral)
	{
		// 两种都有，按权重随机
		bSpawnNeutral = FMath::FRand() < NeutralSpawnWeight;
	}
	else if (bHasNeutral)
	{
		// 只有中立方块
		bSpawnNeutral = true;
	}
	// else: 只有敌人方块，bSpawnNeutral 保持 false

	if (bSpawnNeutral)
	{
		SpawnNeutralBlock(Location);
	}
	else
	{
		SpawnEnemyBlock(Location);
	}
}

void ASHWorldBlockSpawner::SpawnEnemyBlock(const FVector& Location)
{
	USHEnemyBlockData* EnemyData = GetRandomEnemyData();
	if (!EnemyData)
	{
		return;
	}

	int32 SpawnLevel = FMath::RandRange(MinEnemyLevel, MaxEnemyLevel);
	ASHEnemyBlock::SpawnDeferred(GetWorld(), EnemyData, Location, SpawnLevel);
}

void ASHWorldBlockSpawner::SpawnNeutralBlock(const FVector& Location)
{
	USHNeutralBlockData* NeutralData = GetRandomNeutralData();
	if (!NeutralData)
	{
		return;
	}

	int32 SpawnLevel = FMath::RandRange(MinNeutralLevel, MaxNeutralLevel);

	ASHNeutralBlock::SpawnDeferred(GetWorld(), NeutralData, Location, SpawnLevel);
}

ASHGridBase* ASHWorldBlockSpawner::GetTargetGrid()
{
	if (CachedTargetGrid)
	{
		return CachedTargetGrid;
	}

	if (!CachedGameState)
	{
		CachedGameState = GetWorld()->GetGameState<ASHGameStateBase>();
	}

	if (CachedGameState)
	{
		CachedTargetGrid = CachedGameState->GetPlayerGrid();
	}

	return CachedTargetGrid;
}

USHEnemyBlockData* ASHWorldBlockSpawner::GetRandomEnemyData()
{
	TArray<USHEnemyBlockData*> ValidData;
	for (USHEnemyBlockData* Data : EnemyBlockDataList)
	{
		if (Data && Data->EnemyClass)
		{
			ValidData.Add(Data);
		}
	}

	if (ValidData.Num() == 0)
	{
		return nullptr;
	}

	return ValidData[FMath::RandRange(0, ValidData.Num() - 1)];
}

USHNeutralBlockData* ASHWorldBlockSpawner::GetRandomNeutralData()
{
	TArray<USHNeutralBlockData*> ValidData;
	for (USHNeutralBlockData* Data : NeutralBlockDataList)
	{
		if (Data && Data->NeutralClass)
		{
			ValidData.Add(Data);
		}
	}

	if (ValidData.Num() == 0)
	{
		return nullptr;
	}

	return ValidData[FMath::RandRange(0, ValidData.Num() - 1)];
}

TArray<int32> ASHWorldBlockSpawner::SelectRandomLateralOffsets(int32 Count)
{
	TArray<int32> Available;
	TArray<int32> Selected;

	for (int32 i = MinLateralOffset; i <= MaxLateralOffset; i++)
	{
		Available.Add(i);
	}

	Count = FMath::Min(Count, Available.Num());

	for (int32 i = 0; i < Count; i++)
	{
		if (Available.Num() == 0)
		{
			break;
		}

		const int32 RandomIndex = FMath::RandRange(0, Available.Num() - 1);
		Selected.Add(Available[RandomIndex]);
		Available.RemoveAt(RandomIndex);
	}

	return Selected;
}

FVector ASHWorldBlockSpawner::CalculateSpawnLocation(int32 LateralOffset)
{
	ASHGridBase* TargetGrid = GetTargetGrid();
	if (!TargetGrid)
	{
		return FVector::ZeroVector;
	}

	const float CellSize = TargetGrid->CellSize;
	const int32 GridRows = TargetGrid->Rows;
	const int32 GridColumns = TargetGrid->Columns;
	const FVector GridLocation = TargetGrid->GetActorLocation();
	const FVector GridOrigin = TargetGrid->GridOrigin;

	const int32 TotalRows = GridRows + 2;
	const int32 TotalColumns = GridColumns + 2;

	const FVector CenterOffset(-TotalColumns * CellSize * 0.5f, -TotalRows * CellSize * 0.5f, 0.0f);
	const FVector GridBottomLeft = GridLocation + GridOrigin + CenterOffset;

	// Grid 中心格子索引
	const float GridCenterX = TotalColumns * 0.5f;
	const float GridCenterY = TotalRows * 0.5f;

	float SpawnX = 0.0f;
	float SpawnY = 0.0f;

	switch (SpawnDirection)
	{
	case EGridMovementDirection::PositiveX:
		SpawnX = (TotalColumns + SpawnDistanceOffset + 0.5f) * CellSize;
		SpawnY = (GridCenterY + LateralOffset + 0.5f) * CellSize;
		break;

	case EGridMovementDirection::NegativeX:
		SpawnX = (-SpawnDistanceOffset - 0.5f) * CellSize;
		SpawnY = (GridCenterY + LateralOffset + 0.5f) * CellSize;
		break;

	case EGridMovementDirection::PositiveY:
		SpawnX = (GridCenterX + LateralOffset + 0.5f) * CellSize;
		SpawnY = (TotalRows + SpawnDistanceOffset + 0.5f) * CellSize;
		break;

	case EGridMovementDirection::NegativeY:
		SpawnX = (GridCenterX + LateralOffset + 0.5f) * CellSize;
		SpawnY = (-SpawnDistanceOffset - 0.5f) * CellSize;
		break;

	case EGridMovementDirection::None:
	default:
		SpawnX = (TotalColumns + SpawnDistanceOffset + 0.5f) * CellSize;
		SpawnY = (GridCenterY + LateralOffset + 0.5f) * CellSize;
		break;
	}

	return GridBottomLeft + FVector(SpawnX, SpawnY, 0.0f);
}
