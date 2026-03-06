// Sailing Heart


#include "Character/SHCharacterMovementComponent.h"

#include "GameFramework/Character.h"


USHCharacterMovementComponent::USHCharacterMovementComponent()
{
	// 默认启用平底检测
	bUseFlatBaseForFloorChecks = true;

	// 跳跃时保持与移动平台的绑定
	bStayBasedInAir = true;

	// 按照移动方向旋转
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 100000.0f, 0.0f);

	// 限制在平面移动
	bConstrainToPlane = true;
	bSnapToPlaneAtStart = true;

	// 跳跃和重力设置
	GravityScale = 4.0f;
	JumpZVelocity = 1500.0f;
	AirControl = 0.6f;

	// 瞬间加速
	MaxAcceleration = 8192.0f;
}

void USHCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Coyote Time 在服务器和本地控制的客户端上执行
	ACharacter* Owner = GetCharacterOwner();
	if (Owner && (Owner->HasAuthority() || Owner->IsLocallyControlled()))
	{
		UpdateCoyoteTime(DeltaTime);
	}
}

bool USHCharacterMovementComponent::IsInCoyoteTime() const
{
	// Coyote Time 只在离开地面后生效（不在地面上时）
	return bEnableCoyoteTime && !IsMovingOnGround() && CoyoteTimeCounter > 0.f && !bCoyoteTimeJumpUsed;
}

void USHCharacterMovementComponent::ConsumeCoyoteTime()
{
	bCoyoteTimeJumpUsed = true;
	CoyoteTimeCounter = 0.f;
}

bool USHCharacterMovementComponent::FloorSweepTest(FHitResult& OutHit, const FVector& Start, const FVector& End,
	ECollisionChannel TraceChannel, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params,
	const FCollisionResponseParams& ResponseParam) const
{
	bool bBlockingHit = false;

	if (!bUseFlatBaseForFloorChecks)
	{
		// 不使用平底检测时，使用默认的胶囊体 sweep
		bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, GetWorldToGravityTransform(), TraceChannel, CollisionShape, Params, ResponseParam);
	}
	else
	{
		// 使用自定义比例的 Box 进行检测（默认引擎是 0.707f）
		const float CapsuleRadius = CollisionShape.GetCapsuleRadius();
		const float CapsuleHeight = CollisionShape.GetCapsuleHalfHeight();
		const FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(CapsuleRadius * FlatBaseBoxRatio, CapsuleRadius * FlatBaseBoxRatio, CapsuleHeight));

		// 第一次检测：Box 旋转 45 度（角落对齐主轴方向）
		bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat(GetGravityDirection(), UE_PI * 0.25f), TraceChannel, BoxShape, Params, ResponseParam);

		if (!bBlockingHit)
		{
			// 第二次检测：Box 不旋转
			OutHit.Reset(1.f, false);
			bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, GetWorldToGravityTransform(), TraceChannel, BoxShape, Params, ResponseParam);
		}
	}

	return bBlockingHit;
}

float USHCharacterMovementComponent::GetGravityZ() const
{
	float BaseGravity = Super::GetGravityZ();

	if (!bEnableGravityCurve)
	{
		return BaseGravity;
	}

	// 根据垂直速度判断是上升还是下落
	// 获取重力方向上的速度分量
	float VerticalVelocity = FVector::DotProduct(Velocity, -GetGravityDirection());

	if (VerticalVelocity > 0.f)
	{
		// 上升中
		return BaseGravity * RisingGravityScale;
	}
	else
	{
		// 下落中或静止
		return BaseGravity * FallingGravityScale;
	}
}

void USHCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);
	
	// 落地时重置 Coyote Time 状态
	CoyoteTimeCounter = CoyoteTimeDuration;
	bCoyoteTimeJumpUsed = false;
}

void USHCharacterMovementComponent::UpdateCoyoteTime(float DeltaTime)
{
	if (!bEnableCoyoteTime)
	{
		return;
	}

	bool bIsOnGround = IsMovingOnGround();

	if (bIsOnGround)
	{
		// 在地面上，重置计时器
		CoyoteTimeCounter = CoyoteTimeDuration;
		bCoyoteTimeJumpUsed = false;
	}
	else if (bWasOnGroundLastFrame && !bIsOnGround)
	{
		// 刚离开地面
		// 检查是否是因为跳跃（上升中）
		float VerticalVelocity = FVector::DotProduct(Velocity, -GetGravityDirection());
		if (VerticalVelocity > 0.f)
		{
			// 正在上升，说明是跳跃离开地面
			bCoyoteTimeJumpUsed = true;
			CoyoteTimeCounter = 0.f;
		}
		else
		{
			// 不是跳跃（走下平台），开始计时
			CoyoteTimeCounter = CoyoteTimeDuration;
		}
	}
	else if (!bIsOnGround && !bCoyoteTimeJumpUsed)
	{
		// 在空中且还没跳跃
		// 检查是否开始上升（Coyote Time 跳跃成功）
		float VerticalVelocity = FVector::DotProduct(Velocity, -GetGravityDirection());
		if (VerticalVelocity > 0.f)
		{
			// 开始上升，Coyote Time 跳跃成功，消耗它
			bCoyoteTimeJumpUsed = true;
			CoyoteTimeCounter = 0.f;
		}
		else if (CoyoteTimeCounter > 0.f)
		{
			// 还在下落，继续倒计时
			CoyoteTimeCounter -= DeltaTime;
		}
	}

	bWasOnGroundLastFrame = bIsOnGround;
}
