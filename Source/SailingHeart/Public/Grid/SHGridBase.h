// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SHGridBase.generated.h"

class ASHPlayerBlock;
class ASHGameStateBase;

// Grid 所有者类型
UENUM(BlueprintType)
enum class EGridOwnerType : uint8
{
	None		UMETA(DisplayName = "None"),
	Player		UMETA(DisplayName = "Player"),
	Neutral		UMETA(DisplayName = "Neutral")
};

// Grid 移动方向
UENUM(BlueprintType)
enum class EGridMovementDirection : uint8
{
	None		UMETA(DisplayName = "None"),
	PositiveX	UMETA(DisplayName = "+X (Forward)"),
	NegativeX	UMETA(DisplayName = "-X (Backward)"),
	PositiveY	UMETA(DisplayName = "+Y (Right)"),
	NegativeY	UMETA(DisplayName = "-Y (Left)")
};

UCLASS()
class SAILINGHEART_API ASHGridBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASHGridBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Grid 所有者类型（用于标识玩家Grid）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	EGridOwnerType GridOwnerType = EGridOwnerType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Rows = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Columns = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float LineThickness = 5.0f;

	// Border 蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	TSubclassOf<AActor> MeshBorderClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	TSubclassOf<AActor> DecalBorderClass;

	// Border Actor 引用（本地生成，不需要网络复制）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Border")
	AActor* MeshBorderActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Border")
	AActor* DecalBorderActor;

	UFUNCTION(CallInEditor, Category = "Grid")
	void GenerateGrid();

	// 更新 Border 尺寸的函数接口 - 在蓝图中实现，使用 GetPlaceableWidth/Height 获取尺寸
	UFUNCTION(BlueprintNativeEvent, Category = "Border")
	void UpdateBorderSize();
	virtual void UpdateBorderSize_Implementation();

	// Border 初始化接口 - 生成时只调用一次，用于设置其他参数
	UFUNCTION(BlueprintNativeEvent, Category = "Border")
	void InitializeBorder();
	virtual void InitializeBorder_Implementation();

	// 获取 Grid 总尺寸（包含边缘玩家区域）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	float GetGridWidth() const { return (Columns + 2) * CellSize; }

	UFUNCTION(BlueprintCallable, Category = "Grid")
	float GetGridHeight() const { return (Rows + 2) * CellSize; }

	// 获取实际行列数（包含边缘）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	int32 GetTotalRows() const { return Rows + 2; }

	UFUNCTION(BlueprintCallable, Category = "Grid")
	int32 GetTotalColumns() const { return Columns + 2; }

	// Debug 显示开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugSphere = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugSphereRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FColor DebugSphereColor = FColor::Green;

	// 根据世界坐标获取单元格索引 (Row, Column)，如果在 Grid 外返回 false
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool GetCellIndexFromLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const;

	// 根据单元格索引获取单元格中心的世界坐标
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GetCellCenterLocation(const int32 Row, const int32 Column) const;

	// 检查单元格是否在Grid范围内
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsCellValid(int32 Row, int32 Column) const;

	// 检查单元格是否可以放置方块（排除边缘一圈）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsCellPlaceable(int32 Row, int32 Column) const;

	// 获取可放置区域的尺寸（Border 尺寸，等于设置的行列数）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	float GetPlaceableWidth() const { return Columns * CellSize; }

	UFUNCTION(BlueprintCallable, Category = "Grid")
	float GetPlaceableHeight() const { return Rows * CellSize; }

	// 检查世界坐标是否在此Grid范围内
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool ContainsWorldLocation(const FVector& WorldLocation) const;

	// 静态函数：根据世界坐标找到包含该位置的 GridActor
	UFUNCTION(BlueprintCallable, Category = "Grid", meta = (WorldContext = "WorldContextObject"))
	static ASHGridBase* FindGridAtLocation(const UObject* WorldContextObject, const FVector& WorldLocation);

	// 静态函数：计算玩家面朝方向的单元格位置，并找到对应的 Grid
	// 返回包含该单元格的 GridActor，如果没有找到返回 nullptr
	UFUNCTION(BlueprintCallable, Category = "Grid", meta = (WorldContext = "WorldContextObject"))
	static ASHGridBase* GetPlayerFacingCellInfo(const UObject* WorldContextObject, APawn* PlayerPawn, FVector& OutLocation, int32& OutRow, int32& OutColumn);

	// ========== 单元格船方块管理 ==========

	UFUNCTION(BlueprintCallable, Category = "Grid")
	ASHPlayerBlock* GetShipBlockAt(int32 Row, int32 Column) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	virtual void SetShipBlockAt(int32 Row, int32 Column, ASHPlayerBlock* Block);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	virtual void ClearShipBlockAt(int32 Row, int32 Column);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool HasShipBlockAt(int32 Row, int32 Column) const;

	// ========== Grid 移动 ==========

	// 是否启用 Grid 移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Grid Movement")
	bool bEnableMovement = false;

	// 移动速度（单位/秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Grid Movement")
	float MovementSpeed = 50.0f;

	// 移动方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Grid Movement")
	EGridMovementDirection MovementDirection = EGridMovementDirection::PositiveX;

	// 获取移动方向的单位向量
	UFUNCTION(BlueprintCallable, Category = "Grid Movement")
	FVector GetMovementDirectionVector() const;

	// 运行时启用/禁用移动
	UFUNCTION(BlueprintCallable, Category = "Grid Movement")
	void SetMovementEnabled(bool bEnabled);

	// 运行时设置移动速度
	UFUNCTION(BlueprintCallable, Category = "Grid Movement")
	void SetMovementSpeed(float NewSpeed);

	// 运行时设置移动方向
	UFUNCTION(BlueprintCallable, Category = "Grid Movement")
	void SetMovementDirection(EGridMovementDirection NewDirection);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* GridMesh;

	// 缓存 GameState 指针（避免每帧查找）
	UPROPERTY()
	ASHGameStateBase* CachedGameState;

	void CreateGridLines();
	void AddLine(const FVector& Start, const FVector& End, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals);

	void SpawnBorders();
	void UpdateGridBorders();

	// 初始化单元格碰撞盒
	void InitializeCellCollisions();

	// 启用/禁用单元格碰撞盒（本地）
	void EnableCellCollisionLocal(int32 Row, int32 Column, bool bEnable);

	// Multicast RPC - 同步碰撞盒状态到所有客户端
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEnableCellCollision(int32 Row, int32 Column, bool bEnable);

	// 单元格数据 - Key: FIntPoint(Column, Row), Value: ShipBlockActor（服务器端维护）
	TMap<FIntPoint, ASHPlayerBlock*> CellData;

	// 单元格碰撞盒 - Key: FIntPoint(Column, Row), Value: BoxComponent
	UPROPERTY()
	TMap<FIntPoint, UBoxComponent*> CellCollisions;

	// 有方块的单元格列表（用于网络复制，解决中途加入的客户端同步问题）
	UPROPERTY(ReplicatedUsing = OnRep_OccupiedCells)
	TArray<FIntPoint> OccupiedCells;
	
	UFUNCTION()
	void OnRep_OccupiedCells();
	
	// 根据 OccupiedCells 同步所有碰撞状态
	void SyncAllCellCollisions();
};
