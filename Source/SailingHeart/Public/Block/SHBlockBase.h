// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameplayTagContainer.h"
#include "SHBlockBase.generated.h"

/**
 * 方块基类 - 所有方块类型的基础
 * 包含：碰撞盒、基础组件、阵营标签
 * 不包含 GAS 系统（GAS 在 CombatBlockBase 中）
 */
UCLASS(Abstract)
class SAILINGHEART_API ASHBlockBase : public AActor
{
	GENERATED_BODY()

public:
	ASHBlockBase();

	// ========== 基础访问器 ==========

	UFUNCTION(BlueprintCallable, Category = "Block")
	FName GetBlockTypeID() const { return BlockTypeID; }

	// ========== 组件访问器 ==========

	UFUNCTION(BlueprintCallable, Category = "Components")
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

	UFUNCTION(BlueprintCallable, Category = "Components")
	UStaticMesh* GetMesh() const { return MeshComponent ? MeshComponent->GetStaticMesh() : nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Components")
	UBoxComponent* GetBlockCollisionBox() const { return BlockCollisionBox; }

	// ========== 调试 ==========

	// 设置全局调试显示状态
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void SetGlobalShowDebugAttributes(bool bShow);

	// 获取全局调试显示状态
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static bool GetGlobalShowDebugAttributes() { return bGlobalShowDebugInfo; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========== 组件 ==========

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// 方块碰撞盒（用于方块间碰撞检测）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BlockCollisionBox;

	// StateTree 组件（用于 AI 行为控制）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStateTreeComponent* StateTreeComponent;

	// ========== 碰撞配置 ==========

	// 碰撞盒基于的单元格大小（应与 Grid 的 CellSize 一致）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision")
	float BlockCellSize = 200.0f;

	// 碰撞盒相对于 CellSize 的比例（小于1避免擦边）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision")
	float CollisionSizeRatio = 0.9f;

	// 碰撞预设名称
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision")
	FName BlockCollisionProfileName = TEXT("BlockDefault");

	// 初始化碰撞盒大小和位置（不启用碰撞）
	void SetupBlockCollision();

	// 启用碰撞检测（在初始化完成后调用）
	void EnableBlockCollision();

	// ========== 核心数据 ==========

	// 方块类型 ID
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Block")
	FName BlockTypeID;

	// 阵营标签（子类在构造函数中设置，不可在蓝图中修改）
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Faction")
	FGameplayTag FactionTag;

	// 绘制调试信息（子类覆盖）
	virtual void DrawDebugInfo() {}

	// 获取 StateTree 调试信息
	FString GetStateTreeDebugText() const;

private:
	// 全局调试显示状态
	static bool bGlobalShowDebugInfo;
};
