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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual FVector ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const override;
};
