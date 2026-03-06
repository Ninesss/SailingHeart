// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SHProjectileMovementComponent.generated.h"

/**
 * 自定义投射物移动组件 - 支持 XY 平面 Homing
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class SAILINGHEART_API USHProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	/** 是否受 GlobalTimeScale 影响 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeScale")
	bool bUseGlobalTimeScale = true;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual FVector ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const override;

private:
	float GetGlobalTimeScale() const;
};
