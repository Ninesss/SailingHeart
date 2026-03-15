// Sailing Heart

#include "Projectile/SHProjectileBase.h"
#include "Projectile/SHProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "Interface/SHCombatInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Game/SHGameStateBase.h"

ASHProjectileBase::ASHProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(20.f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetGenerateOverlapEvents(true);
	SetRootComponent(CollisionComponent);

	ProjectileMovement = CreateDefaultSubobject<USHProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->SetIsReplicated(true);

	// 投射物视觉效果（在蓝图中设置 Asset）
	ProjectileVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileVFX"));
	ProjectileVFX->SetupAttachment(CollisionComponent);
	ProjectileVFX->bAutoActivate = true;
}

void ASHProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASHProjectileBase, HomingTarget, COND_None);
}

void ASHProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpan);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASHProjectileBase::OnProjectileOverlap);
	PlaySpawnFX();

	if (LoopingSound)
	{
		LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(
			LoopingSound,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 绑定 GlobalTimeScale 事件
	if (UWorld* World = GetWorld())
	{
		CachedGameState = World->GetGameState<ASHGameStateBase>();
		if (CachedGameState.IsValid())
		{
			CachedGameState->OnGlobalTimeScaleChanged.AddDynamic(this, &ASHProjectileBase::OnGlobalTimeScaleChanged);
			// 初始化时设置一次
			OnGlobalTimeScaleChanged(CachedGameState->GetGlobalTimeScale());
		}
	}
}

void ASHProjectileBase::Destroyed()
{
	UnbindHomingTargetEvents();

	// 解绑 GlobalTimeScale 事件
	if (CachedGameState.IsValid())
	{
		CachedGameState->OnGlobalTimeScaleChanged.RemoveDynamic(this, &ASHProjectileBase::OnGlobalTimeScaleChanged);
	}

	if (IsValid(LoopingSoundComponent))
	{
		LoopingSoundComponent->Stop();
	}
	Super::Destroyed();
}

void ASHProjectileBase::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	AActor* InstigatorActor = GetInstigator();
	AActor* OwnerActor = GetOwner();
	if (OtherActor == InstigatorActor || OtherActor == OwnerActor)
	{
		return;
	}

	// 阵营判断使用 Owner 兜底（方块不是 Pawn，Instigator 可能为空）
	AActor* SourceActor = InstigatorActor ? InstigatorActor : OwnerActor;
	if (USHAbilitySystemLibrary::AreActorsFriends(SourceActor, OtherActor))
	{
		return;
	}

	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	if (HasAuthority() && !bHit)
	{
		HandleHitTarget(OtherActor, SweepResult);
	}
}

void ASHProjectileBase::HandleHitTarget(AActor* HitActor, const FHitResult& HitResult)
{
	HitActors.Add(HitActor);
	ApplyDamageToTarget(HitActor);
	ApplyHitEffectsToTarget(HitActor);

	if (bPenetrating)
	{
		PenetrationCount++;
		if (MaxPenetrations > 0 && PenetrationCount >= MaxPenetrations)
		{
			bHit = true;
			MulticastPlayImpactFX(GetActorLocation(), GetActorRotation());
			OnProjectileDestroyed();
			Destroy();
		}
	}
	else
	{
		bHit = true;
		MulticastPlayImpactFX(GetActorLocation(), GetActorRotation());
		OnProjectileDestroyed();
		Destroy();
	}
}

void ASHProjectileBase::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!TargetActor || !DamageEffectSpecHandle.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC)
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	}
}

void ASHProjectileBase::ApplyHitEffectsToTarget(AActor* TargetActor)
{
	if (!TargetActor || HitEffectSpecHandles.Num() == 0)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC)
	{
		for (const FGameplayEffectSpecHandle& SpecHandle : HitEffectSpecHandles)
		{
			if (SpecHandle.IsValid())
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void ASHProjectileBase::OnProjectileDestroyed_Implementation()
{
}

void ASHProjectileBase::PlaySpawnFX()
{
	const FVector Location = GetActorLocation();
	const FRotator Rotation = GetActorRotation();

	if (SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SpawnEffect, Location, Rotation);
	}

	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, Location);
	}
}

void ASHProjectileBase::MulticastPlayImpactFX_Implementation(const FVector& Location, const FRotator& Rotation)
{
	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, Location, Rotation);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
	}

	if (IsValid(LoopingSoundComponent))
	{
		LoopingSoundComponent->Stop();
	}
}

void ASHProjectileBase::SetHomingTarget(AActor* NewTarget)
{
	if (!HasAuthority())
	{
		return;
	}

	UnbindHomingTargetEvents();

	HomingTarget = NewTarget;

	if (NewTarget)
	{
		// 目标已死（但尚未 Destroy）时，立刻寻找下一个活着的目标
		if (NewTarget->Implements<USHCombatInterface>() && ISHCombatInterface::Execute_IsDead(NewTarget))
		{
			AActor* LiveTarget = FindNearestEnemy(NewTarget);
			HomingTarget = LiveTarget;
			NewTarget = LiveTarget;
		}
	}

	if (NewTarget)
	{
		BindHomingTargetEvents(NewTarget);

		if (ProjectileMovement)
		{
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingTargetComponent = NewTarget->GetRootComponent();
		}
	}
	else
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->HomingTargetComponent = nullptr;
		}
	}
}

void ASHProjectileBase::OnRep_HomingTarget()
{
	// 客户端收到新目标后更新 ProjectileMovement
	if (ProjectileMovement)
	{
		if (HomingTarget.IsValid())
		{
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->HomingTargetComponent = nullptr;
		}
	}
}

void ASHProjectileBase::OnHomingTargetDied(AActor* DeadActor)
{
	if (!HasAuthority() || !bAutoRetarget)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Projectile] 目标 %s 已死亡，立刻寻找新目标"), *GetNameSafe(DeadActor));

	AActor* NewTarget = FindNearestEnemy(DeadActor);
	if (NewTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("[Projectile] 找到新目标: %s"), *GetNameSafe(NewTarget));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Projectile] 未找到新目标，投射物将直线飞行"));
	}

	SetHomingTarget(NewTarget);
}

void ASHProjectileBase::OnHomingTargetDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority() || !bAutoRetarget)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Projectile] 目标 %s 已销毁，开始寻找新目标"), *GetNameSafe(DestroyedActor));

	// 目标销毁，寻找新目标（排除正在销毁的 Actor）
	AActor* NewTarget = FindNearestEnemy(DestroyedActor);

	if (NewTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("[Projectile] 找到新目标: %s"), *GetNameSafe(NewTarget));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Projectile] 未找到新目标，投射物将直线飞行"));
	}

	SetHomingTarget(NewTarget);
}

AActor* ASHProjectileBase::FindNearestEnemy(AActor* ExcludeActor) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Origin = GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams(FCollisionObjectQueryParams::AllDynamicObjects);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(RetargetSearchRange);

	World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		ObjectParams,
		SphereShape
	);

	AActor* SourceActor = GetInstigator() ? GetInstigator() : GetOwner();
	AActor* NearestEnemy = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Actor == SourceActor || Actor == ExcludeActor || HitActors.Contains(Actor))
		{
			continue;
		}

		if (!USHAbilitySystemLibrary::AreActorsEnemies(SourceActor, Actor))
		{
			continue;
		}

		if (Actor->Implements<USHCombatInterface>() && ISHCombatInterface::Execute_IsDead(Actor))
		{
			continue;
		}

		float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestEnemy = Actor;
		}
	}

	return NearestEnemy;
}

void ASHProjectileBase::BindHomingTargetEvents(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	Target->OnDestroyed.AddDynamic(this, &ASHProjectileBase::OnHomingTargetDestroyed);

	// 订阅 OnDeath delegate，在 Health 归零时立刻触发重寻，不等 Actor 真正 Destroy
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
	{
		if (USHAttributeSetBase* TargetAttrSet = const_cast<USHAttributeSetBase*>(TargetASC->GetSet<USHAttributeSetBase>()))
		{
			TargetAttrSet->OnDeath.AddDynamic(this, &ASHProjectileBase::OnHomingTargetDied);
		}
	}
}

void ASHProjectileBase::UnbindHomingTargetEvents()
{
	if (!HomingTarget.IsValid())
	{
		return;
	}

	HomingTarget->OnDestroyed.RemoveDynamic(this, &ASHProjectileBase::OnHomingTargetDestroyed);

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HomingTarget.Get()))
	{
		if (USHAttributeSetBase* TargetAttrSet = const_cast<USHAttributeSetBase*>(TargetASC->GetSet<USHAttributeSetBase>()))
		{
			TargetAttrSet->OnDeath.RemoveDynamic(this, &ASHProjectileBase::OnHomingTargetDied);
		}
	}
}

void ASHProjectileBase::OnGlobalTimeScaleChanged(float NewTimeScale)
{
	if (ProjectileVFX)
	{
		ProjectileVFX->SetCustomTimeDilation(NewTimeScale);
	}
}
