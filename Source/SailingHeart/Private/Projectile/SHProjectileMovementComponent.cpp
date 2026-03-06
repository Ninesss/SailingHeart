// Sailing Heart

#include "Projectile/SHProjectileMovementComponent.h"
#include "Projectile/SHProjectileBase.h"
#include "Game/SHGameStateBase.h"

void USHProjectileMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	const float ScaledDeltaTime = bUseGlobalTimeScale ? DeltaTime * GetGlobalTimeScale() : DeltaTime;

	// 从 Owner Projectile 读取 bHomingXYOnly
	bool bHomingXYOnly = false;
	if (const ASHProjectileBase* Projectile = Cast<ASHProjectileBase>(GetOwner()))
	{
		bHomingXYOnly = Projectile->bHomingXYOnly;
	}

	// 如果启用 XY 平面追踪，在 Tick 前记录初始 Z 位置
	const float InitialZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : 0.f;

	Super::TickComponent(ScaledDeltaTime, TickType, ThisTickFunction);

	// 如果启用 XY 平面追踪，强制保持 Z 位置不变
	if (bHomingXYOnly && bIsHomingProjectile && UpdatedComponent)
	{
		FVector Location = UpdatedComponent->GetComponentLocation();
		if (!FMath::IsNearlyEqual(Location.Z, InitialZ, 1.f))
		{
			Location.Z = InitialZ;
			UpdatedComponent->SetWorldLocation(Location);

			// 同时清除 Z 方向的速度
			Velocity.Z = 0.f;
		}
	}
}

float USHProjectileMovementComponent::GetGlobalTimeScale() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const ASHGameStateBase* GS = World->GetGameState<ASHGameStateBase>())
		{
			return GS->GetGlobalTimeScale();
		}
	}
	return 1.0f;
}

FVector USHProjectileMovementComponent::ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	if (!HomingTargetComponent.IsValid() || !UpdatedComponent)
	{
		return FVector::ZeroVector;
	}

	FVector Direction = HomingTargetComponent->GetComponentLocation() - UpdatedComponent->GetComponentLocation();

	// 从 Owner Projectile 读取 bHomingXYOnly
	if (const ASHProjectileBase* Projectile = Cast<ASHProjectileBase>(GetOwner()))
	{
		if (Projectile->bHomingXYOnly)
		{
			Direction.Z = 0.f;
		}
	}

	return Direction.GetSafeNormal() * HomingAccelerationMagnitude;
}
