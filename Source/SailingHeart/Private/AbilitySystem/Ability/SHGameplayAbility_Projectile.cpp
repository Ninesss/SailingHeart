// Sailing Heart

#include "AbilitySystem/Ability/SHGameplayAbility_Projectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "Projectile/SHProjectileMovementComponent.h"
#include "Data/Ability/SHAbilityParams.h"
#include "Data/Ability/SHProjectileAbilityData.h"
#include "Engine/OverlapResult.h"

USHGameplayAbility_Projectile::USHGameplayAbility_Projectile()
{
	// 投射物生成只在服务器执行
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

// ========== DataAsset 获取 ==========

const FSHProjectileAbilityParams* USHGameplayAbility_Projectile::GetCurrentParams() const
{
	USHProjectileAbilityData* Data = Cast<USHProjectileAbilityData>(GetAbilityData());
	if (Data)
	{
		return Data->GetParams(GetAbilityLevel());
	}
	UE_LOG(LogTemp, Warning, TEXT("USHGameplayAbility_Projectile::GetCurrentParams - No DataAsset found for %s"), *GetName());
	return nullptr;
}

// ========== 参数获取（从 DataAsset 读取）==========

TSubclassOf<UGameplayEffect> USHGameplayAbility_Projectile::GetDamageEffectClassParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->DamageEffectClass;
	}
	return nullptr;
}

FGameplayTagContainer USHGameplayAbility_Projectile::GetDamageTypeTagsParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->DamageTypeTags;
	}
	return FGameplayTagContainer();
}

float USHGameplayAbility_Projectile::GetDamageMultiplierParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->DamageMultiplier;
	}
	return 1.0f;
}

TSubclassOf<ASHProjectileBase> USHGameplayAbility_Projectile::GetProjectileClassParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->ProjectileClass;
	}
	return nullptr;
}

float USHGameplayAbility_Projectile::GetProjectileSpeedParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->ProjectileSpeed;
	}
	return 2000.f;
}

float USHGameplayAbility_Projectile::GetLifeSpanParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->LifeSpan;
	}
	return 10.f;
}

FVector USHGameplayAbility_Projectile::GetSpawnOffsetParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->SpawnOffset;
	}
	return FVector(100.f, 0.f, 0.f);
}

int32 USHGameplayAbility_Projectile::GetProjectilesPerActivationParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->ProjectilesPerActivation;
	}
	return 1;
}

float USHGameplayAbility_Projectile::GetSpreadAngleParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->SpreadAngle;
	}
	return 15.f;
}

bool USHGameplayAbility_Projectile::GetPenetratingParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->bPenetrating;
	}
	return false;
}

int32 USHGameplayAbility_Projectile::GetMaxPenetrationsParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->MaxPenetrations;
	}
	return 0;
}

TArray<FSHProjectileHitEffect> USHGameplayAbility_Projectile::GetHitEffectsParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->HitEffects;
	}
	return TArray<FSHProjectileHitEffect>();
}

// 目标和移动类型参数
float USHGameplayAbility_Projectile::GetTargetSearchRangeParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->TargetSearchRange;
	}
	return 0.f;
}

float USHGameplayAbility_Projectile::GetRetargetSearchRangeParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		// 如果 RetargetSearchRange 为 0，则使用 TargetSearchRange
		return Params->RetargetSearchRange > 0.f ? Params->RetargetSearchRange : Params->TargetSearchRange;
	}
	return 0.f;
}

EProjectileMovementType USHGameplayAbility_Projectile::GetMovementTypeParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->MovementType;
	}
	return EProjectileMovementType::Linear;
}

float USHGameplayAbility_Projectile::GetHomingAccelerationParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->HomingAcceleration;
	}
	return 5000.f;
}

bool USHGameplayAbility_Projectile::GetHomingXYOnlyParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->bHomingXYOnly;
	}
	return true;
}

bool USHGameplayAbility_Projectile::GetAutoRetargetParam() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->bAutoRetarget;
	}
	return true;
}

// ========== Cost/Cooldown 获取 ==========

float USHGameplayAbility_Projectile::GetEnergyCost() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->EnergyCost;
	}
	return 0.f;
}

float USHGameplayAbility_Projectile::GetAbilityCooldownDuration() const
{
	if (const FSHProjectileAbilityParams* Params = GetCurrentParams())
	{
		return Params->CooldownDuration;
	}
	return 0.f;
}

// ========== ISHDamageAbilityInterface 实现 ==========

TSubclassOf<UGameplayEffect> USHGameplayAbility_Projectile::GetDamageEffectClass_Implementation() const
{
	return GetDamageEffectClassParam();
}

FGameplayTagContainer USHGameplayAbility_Projectile::GetDamageTypeTags_Implementation() const
{
	return GetDamageTypeTagsParam();
}

float USHGameplayAbility_Projectile::GetDamageMultiplier_Implementation() const
{
	return GetDamageMultiplierParam();
}

UAbilitySystemComponent* USHGameplayAbility_Projectile::GetDamageSourceASC_Implementation() const
{
	return GetOwnerASC();
}

AActor* USHGameplayAbility_Projectile::GetDamageSourceActor_Implementation() const
{
	return GetSourceActor();
}

// ========== 目标获取 ==========

AActor* USHGameplayAbility_Projectile::AcquireTarget(const FGameplayEventData* TriggerEventData)
{
	// 1. 优先使用 Event 传入的目标（StateTree 场景）
	if (TriggerEventData && TriggerEventData->Target != nullptr)
	{
		return const_cast<AActor*>(TriggerEventData->Target.Get());
	}

	// 2. 如果配置了自动寻敌范围，则自动寻敌（玩家场景）
	const float SearchRange = GetTargetSearchRangeParam();
	if (SearchRange > 0.f)
	{
		const FVector SpawnLocation = GetSpawnLocation(GetCurrentActorInfo());
		return FindNearestEnemy(SpawnLocation);
	}

	// 3. 无目标
	return nullptr;
}

AActor* USHGameplayAbility_Projectile::FindNearestEnemy(const FVector& Origin) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams(FCollisionObjectQueryParams::AllDynamicObjects);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(GetTargetSearchRangeParam());

	World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		ObjectParams,
		SphereShape
	);

	AActor* SourceActor = GetAvatarActor();
	AActor* NearestEnemy = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Actor == SourceActor)
		{
			continue;
		}

		if (USHAbilitySystemLibrary::AreActorsEnemies(SourceActor, Actor))
		{
			float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestEnemy = Actor;
			}
		}
	}

	return NearestEnemy;
}

// ========== 能力激活 ==========

void USHGameplayAbility_Projectile::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 使用统一的Cost/Cooldown检查（替代CommitAbility）
	if (!CheckAndCommitAbilityCost())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 获取目标（可能为空，根据配置决定是否需要目标）
	CurrentTarget = AcquireTarget(TriggerEventData);

	FireProjectiles(ActorInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void USHGameplayAbility_Projectile::FireProjectiles(const FGameplayAbilityActorInfo* ActorInfo)
{
	TSubclassOf<ASHProjectileBase> ProjClass = GetProjectileClassParam();
	if (!ProjClass || !ActorInfo)
	{
		return;
	}

	const FVector SpawnLocation = GetSpawnLocation(ActorInfo);
	const int32 ProjCount = GetProjectilesPerActivationParam();

	for (int32 i = 0; i < ProjCount; ++i)
	{
		const FRotator SpawnRotation = GetSpawnRotation(ActorInfo, i);
		SpawnProjectile(ActorInfo, SpawnLocation, SpawnRotation);
	}
}

FVector USHGameplayAbility_Projectile::GetSpawnLocation(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		return FVector::ZeroVector;
	}

	return AvatarActor->GetActorLocation() + AvatarActor->GetActorRotation().RotateVector(GetSpawnOffsetParam());
}

FRotator USHGameplayAbility_Projectile::GetSpawnRotation(const FGameplayAbilityActorInfo* ActorInfo, int32 ProjectileIndex) const
{
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		return FRotator::ZeroRotator;
	}

	FRotator BaseRotation;

	// 如果有目标，朝目标方向；否则朝角色前方
	if (CurrentTarget.IsValid())
	{
		const FVector SpawnLocation = GetSpawnLocation(ActorInfo);
		FVector TargetLocation = CurrentTarget->GetActorLocation();

		// 如果启用 XY 平面限制，忽略 Z 轴差异
		if (GetHomingXYOnlyParam())
		{
			TargetLocation.Z = SpawnLocation.Z;
		}

		FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
		BaseRotation = Direction.Rotation();
	}
	else
	{
		BaseRotation = AvatarActor->GetActorRotation();
	}

	// 多发时应用扩散
	const int32 ProjCount = GetProjectilesPerActivationParam();
	const float Spread = GetSpreadAngleParam();

	if (ProjCount > 1 && Spread > 0.f)
	{
		const float TotalSpread = Spread * (ProjCount - 1);
		const float StartAngle = -TotalSpread * 0.5f;
		const float AngleStep = TotalSpread / (ProjCount - 1);
		const float CurrentAngle = StartAngle + AngleStep * ProjectileIndex;

		BaseRotation.Yaw += CurrentAngle;
	}

	return BaseRotation;
}

ASHProjectileBase* USHGameplayAbility_Projectile::SpawnProjectile(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& Location,
	const FRotator& Rotation)
{
	TSubclassOf<ASHProjectileBase> ProjClass = GetProjectileClassParam();
	UWorld* World = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr;
	if (!World || !ProjClass)
	{
		return nullptr;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();

	FTransform SpawnTransform(Rotation, Location);
	ASHProjectileBase* Projectile = World->SpawnActorDeferred<ASHProjectileBase>(
		ProjClass,
		SpawnTransform,
		AvatarActor,
		Cast<APawn>(AvatarActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (Projectile)
	{
		// 配置投射物（子类可重写添加额外配置）
		ConfigureProjectile(Projectile, ActorInfo);

		Projectile->FinishSpawning(SpawnTransform);
	}

	return Projectile;
}

void USHGameplayAbility_Projectile::ConfigureProjectile(
	ASHProjectileBase* Projectile,
	const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!Projectile || !ActorInfo)
	{
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();

	// 配置速度（从 DataAsset 或默认值）
	if (USHProjectileMovementComponent* Movement = Projectile->ProjectileMovement)
	{
		float Speed = GetProjectileSpeedParam();
		Movement->InitialSpeed = Speed;
		Movement->MaxSpeed = Speed;
	}

	// 配置存活时间
	Projectile->LifeSpan = GetLifeSpanParam();

	// 配置穿透（从 DataAsset 或默认值）
	Projectile->bPenetrating = GetPenetratingParam();
	Projectile->MaxPenetrations = GetMaxPenetrationsParam();

	// 配置伤害效果（使用接口函数，会自动从 DataAsset 读取）
	TSubclassOf<UGameplayEffect> DamageGE = GetDamageEffectClassParam();
	if (DamageGE)
	{
		Projectile->DamageEffectSpecHandle = Execute_MakeDamageSpec(const_cast<USHGameplayAbility_Projectile*>(this), 1.f);
	}

	// 配置命中效果（从 DataAsset 读取）
	TArray<FSHProjectileHitEffect> HitEffectsArray = GetHitEffectsParam();
	if (SourceASC && HitEffectsArray.Num() > 0)
	{
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(AvatarActor);

		for (const FSHProjectileHitEffect& HitEffect : HitEffectsArray)
		{
			if (HitEffect.EffectClass)
			{
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
					HitEffect.EffectClass,
					HitEffect.Level,
					ContextHandle
				);
				Projectile->HitEffectSpecHandles.Add(SpecHandle);
			}
		}
	}

	// 根据移动类型配置
	const EProjectileMovementType MovementType = GetMovementTypeParam();

	switch (MovementType)
	{
	case EProjectileMovementType::Homing:
		// 配置 Homing（需要有目标）
		if (CurrentTarget.IsValid())
		{
			Projectile->bHomingXYOnly = GetHomingXYOnlyParam();
			Projectile->bAutoRetarget = GetAutoRetargetParam();
			Projectile->RetargetSearchRange = GetRetargetSearchRangeParam();

			if (UProjectileMovementComponent* Movement = Projectile->ProjectileMovement)
			{
				Movement->HomingAccelerationMagnitude = GetHomingAccelerationParam();
			}

			// 使用 SetHomingTarget 以绑定目标销毁事件
			Projectile->SetHomingTarget(CurrentTarget.Get());
		}
		break;

	case EProjectileMovementType::Parabolic:
		// TODO: 抛物线移动实现
		break;

	case EProjectileMovementType::Linear:
	default:
		// 直线飞行，无需额外配置（已通过 GetSpawnRotation 设置方向）
		break;
	}
}
