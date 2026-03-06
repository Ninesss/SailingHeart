// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SHCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SAILINGHEART_API USHCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	USHCharacterMovementComponent();
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// ========== 平底检测 ==========
	
	// 平底检测的 Box 比例（相对于胶囊体半径），默认引擎是 0.707f
	// 范围：0.1 ~ 0.707，越小越不容易从边缘滑落，但检测区域也越小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Floor", meta = (ClampMin = "0.1", ClampMax = "0.707"))
	float FlatBaseBoxRatio = 0.5f;
	
	// ========== Coyote Time（悬崖宽容时间）==========

	// 是否启用 Coyote Time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Coyote Time")
	bool bEnableCoyoteTime = true;

	// Coyote Time 时长（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Coyote Time", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CoyoteTimeDuration = 0.15f;

	// 是否在 Coyote Time 内
	UFUNCTION(BlueprintCallable, Category = "Character Movement: Coyote Time")
	bool IsInCoyoteTime() const;

	// 标记 Coyote Time 跳跃已使用（跳跃时调用）
	UFUNCTION(BlueprintCallable, Category = "Character Movement: Coyote Time")
	void ConsumeCoyoteTime();

	// ========== 重力曲线（上升/下落不同重力）==========

	// 是否启用重力曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Gravity Curve")
	bool bEnableGravityCurve = true;

	// 上升时的重力倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Gravity Curve", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float RisingGravityScale = 1.5f;

	// 下落时的重力倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Gravity Curve", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float FallingGravityScale = 3.0f;

protected:
	// 重写地板 Sweep 检测
	virtual bool FloorSweepTest(
		FHitResult& OutHit,
		const FVector& Start,
		const FVector& End,
		ECollisionChannel TraceChannel,
		const FCollisionShape& CollisionShape,
		const FCollisionQueryParams& Params,
		const FCollisionResponseParams& ResponseParam
	) const override;

	// 重写重力计算
	virtual float GetGravityZ() const override;

	// 重写落地处理
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

private:
	// Coyote Time 计时器
	float CoyoteTimeCounter = 0.f;

	// 上一帧是否在地面上
	bool bWasOnGroundLastFrame = false;

	// 是否已经使用了 Coyote Time 跳跃
	bool bCoyoteTimeJumpUsed = false;

	// 更新 Coyote Time 状态
	void UpdateCoyoteTime(float DeltaTime);

};
