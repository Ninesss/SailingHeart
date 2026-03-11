// Sailing Heart

#include "AbilitySystem/Ability/SHGameplayAbility_Projectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "Projectile/SHProjectileMovementComponent.h"
#include "Data/Ability/SHAbilityParams.h"
#include "Data/Ability/SHProjectileAbilityData.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "Engine/OverlapResult.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Interface/SHCombatInterface.h"
#include "Block/SHBlockBase.h"
#include "Block/SHCombatBlockBase.h"
#include "SHGameplayTags.h"

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

// ========== 能力激活（两段式：播放动画 → Notify → 发射）==========

void USHGameplayAbility_Projectile::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 检查冷却 + 消耗（此处 Commit，不在 Notify 回调里再次 Commit）
	if (!CheckAndCommitAbilityCost())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 缓存目标（可能为空）
	CurrentTarget = AcquireTarget(TriggerEventData);

	// 尝试播放 Montage 并等待 AnimNotify 触发
	// 如果没有配置 Montage，直接退化为立即发射（保持向后兼容）
	PlayMontageAndWaitForEvent(ActorInfo);
}

void USHGameplayAbility_Projectile::PlayMontageAndWaitForEvent(const FGameplayAbilityActorInfo* ActorInfo)
{
	AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!AvatarActor)
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 通过接口从 Avatar 获取对应 TriggerTag 的 Montage
	// Avatar（蓝图子类）在 AbilityMontageMap 中配置，技能本身不保存 Montage
	UAnimMontage* Montage = nullptr;
	if (AvatarActor->Implements<USHCombatInterface>())
	{
		if (USHAbilityDataBase* Data = Cast<USHAbilityDataBase>(GetAbilityData()))
		{
			if (Data->TriggerTag.IsValid())
			{
				Montage = ISHCombatInterface::Execute_GetAbilityMontage(AvatarActor, Data->TriggerTag);
			}
		}
	}

	if (!Montage)
	{
		// 无 Montage 配置：立即发射（兼容未设置动画的老数据）
		FireProjectiles(ActorInfo);
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 监听 AnimNotify 发送的 Socket 事件（CombatSocket.Weapon）
	// USHAbilityAnimNotify 在关键帧时发送此 Tag，携带 Socket 位置
	// 注意：这里的 Tag 与激活技能的 TriggerTag（Ability.Trigger.*）不同
	const FGameplayTag EventTag = FSHGameplayTags::Get().CombatSocket_Weapon;

	// 注册 WaitGameplayEvent（一次性监听，收到后触发回调）
	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		EventTag,
		nullptr,    // 不限制 Instigator
		true,       // bTriggerOnce
		true        // bMatchExact
	);
	WaitTask->EventReceived.AddDynamic(this, &USHGameplayAbility_Projectile::OnAnimNotifyEvent);
	WaitTask->ReadyForActivation();

	// 播放 Montage（使用 FunctionalSKM 上的 AnimInstance）
	// 方块的 SKM AnimInstance 直接播放 Montage
	if (ASHCombatBlockBase* Block = Cast<ASHCombatBlockBase>(AvatarActor))
	{
		USkeletalMeshComponent* SKM = Block->GetFunctionalSKM();
		UAnimInstance* AnimInstance = SKM ? SKM->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			// Multicast 广播到所有客户端，各自在本地播放
			Block->Multicast_PlayMontage(Montage);
			return;
		}
		// SKM 未配置 AnimBP，等待配置后再触发
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 角色类型：使用 UAbilityTask_PlayMontageAndWait
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage
	);
	MontageTask->ReadyForActivation();
}

void USHGameplayAbility_Projectile::OnAnimNotifyEvent(FGameplayEventData Payload)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 从 Payload.TargetData 提取 Socket 世界位置（由 USHAbilityAnimNotify 打包）
	FVector SocketLocation = FVector::ZeroVector;
	if (Payload.TargetData.Num() > 0)
	{
		const FGameplayAbilityTargetData_LocationInfo* LocationData =
			static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Payload.TargetData.Get(0));
		if (LocationData)
		{
			SocketLocation = LocationData->TargetLocation.GetTargetingTransform().GetLocation();
		}
	}

	if (!SocketLocation.IsZero())
	{
		FireProjectilesFromSocket(SocketLocation);
	}
	else
	{
		// Notify 没有提供位置时退化为默认位置
		FireProjectiles(ActorInfo);
	}

	EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
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

void USHGameplayAbility_Projectile::FireProjectilesFromSocket(const FVector& SocketLocation)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	TSubclassOf<ASHProjectileBase> ProjClass = GetProjectileClassParam();
	if (!ProjClass || !ActorInfo)
	{
		return;
	}

	const int32 ProjCount = GetProjectilesPerActivationParam();
	for (int32 i = 0; i < ProjCount; ++i)
	{
		const FRotator SpawnRotation = GetSpawnRotationFromSocket(SocketLocation, i);
		SpawnProjectile(ActorInfo, SocketLocation, SpawnRotation);
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

	const FVector SpawnLocation = GetSpawnLocation(ActorInfo);
	return GetSpawnRotationFromSocket(SpawnLocation, ProjectileIndex);
}

FRotator USHGameplayAbility_Projectile::GetSpawnRotationFromSocket(const FVector& SocketLocation, int32 ProjectileIndex) const
{
	FRotator BaseRotation;

	if (CurrentTarget.IsValid())
	{
		FVector TargetLocation = CurrentTarget->GetActorLocation();
		if (GetHomingXYOnlyParam())
		{
			TargetLocation.Z = SocketLocation.Z;
		}
		BaseRotation = (TargetLocation - SocketLocation).GetSafeNormal().Rotation();
	}
	else
	{
		// 无目标时使用 Avatar 朝向
		const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
		AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
		BaseRotation = AvatarActor ? AvatarActor->GetActorRotation() : FRotator::ZeroRotator;
	}

	// 多发扩散
	const int32 ProjCount = GetProjectilesPerActivationParam();
	const float Spread = GetSpreadAngleParam();
	if (ProjCount > 1 && Spread > 0.f)
	{
		const float TotalSpread = Spread * (ProjCount - 1);
		const float StartAngle = -TotalSpread * 0.5f;
		const float AngleStep = TotalSpread / (ProjCount - 1);
		BaseRotation.Yaw += StartAngle + AngleStep * ProjectileIndex;
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
