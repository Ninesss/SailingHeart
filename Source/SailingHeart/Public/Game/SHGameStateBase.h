// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SHGameStateBase.generated.h"

class ASHGridBase;
class USHPlayerBlockData;
class USHPlayerCharacterData;

// GlobalTimeScale 变化时广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalTimeScaleChanged, float, NewTimeScale);

/**
 *
 */
UCLASS()
class SAILINGHEART_API ASHGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	ASHGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========== 玩家方块数据（DataAsset）==========

	// 玩家方块数据列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Data")
	TArray<USHPlayerBlockData*> PlayerBlockDataList;

	// 根据 BlockTypeID 查找方块数据
	UFUNCTION(BlueprintCallable, Category = "Block Data")
	USHPlayerBlockData* GetPlayerBlockData(FName BlockTypeID) const;

	// ========== 玩家角色数据（DataAsset）==========

	// 可选角色数据列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	TArray<USHPlayerCharacterData*> PlayerCharacterDataList;

	// 根据 CharacterID 查找角色数据
	UFUNCTION(BlueprintCallable, Category = "Character Data")
	USHPlayerCharacterData* GetPlayerCharacterData(FName CharacterID) const;

	// 获取所有可选角色 ID
	UFUNCTION(BlueprintCallable, Category = "Character Data")
	TArray<FName> GetAvailableCharacterIDs() const;

	// ========== 玩家 Grid 管理 ==========

	// 注册玩家Grid（由Grid在BeginPlay时调用）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void RegisterPlayerGrid(ASHGridBase* Grid);

	// 获取玩家Grid
	UFUNCTION(BlueprintCallable, Category = "Grid")
	ASHGridBase* GetPlayerGrid() const { return PlayerGrid; }

	// ========== 全局时间流速 ==========

	// 时间流速变化事件（服务器和客户端都会触发）
	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnGlobalTimeScaleChanged OnGlobalTimeScaleChanged;

	// 获取全局时间流速
	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetGlobalTimeScale() const { return GlobalTimeScale; }

	// 设置全局时间流速（服务器执行）
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetGlobalTimeScale(float NewTimeScale);

	// ========== 时间减缓能力（引用计数）==========

	// 激活时间减缓（引用计数 +1）
	UFUNCTION(BlueprintCallable, Category = "Time")
	void ActivateTimeSlow();

	// 关闭时间减缓（引用计数 -1）
	UFUNCTION(BlueprintCallable, Category = "Time")
	void DeactivateTimeSlow();

	// 是否有玩家在使用时间减缓
	UFUNCTION(BlueprintCallable, Category = "Time")
	bool IsTimeSlowActive() const { return TimeSlowActiveCount > 0; }

	// 减缓后的时间倍率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Time")
	float SlowedTimeScale = 0.1f;

protected:
	// 玩家Grid引用（唯一）
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	ASHGridBase* PlayerGrid = nullptr;

	// 全局时间流速（1.0 = 正常速度，0.5 = 半速，2.0 = 双倍速）
	UPROPERTY(ReplicatedUsing = OnRep_GlobalTimeScale, BlueprintReadOnly, Category = "Time")
	float GlobalTimeScale = 1.0f;

	// 客户端收到同步时调用
	UFUNCTION()
	void OnRep_GlobalTimeScale();

private:
	// 广播时间流速变化事件
	void BroadcastTimeScaleChanged();

	// 时间减缓激活计数（多人同时使用时不会叠加效果）
	int32 TimeSlowActiveCount = 0;
};
