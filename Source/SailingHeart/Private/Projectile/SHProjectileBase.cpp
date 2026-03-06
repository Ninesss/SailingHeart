// Sailing Heart

#include "Projectile/SHProjectileBase.h"
#include "Projectile/SHProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
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
	if (OtherActor == InstigatorActor)
	{
		return;
	}

	if (USHAbilitySystemLibrary::AreActorsFriends(InstigatorActor, OtherActor))
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

	AActor* SourceActor = GetInstigator();
	AActor* NearestEnemy = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Actor == SourceActor || Actor == ExcludeActor || HitActors.Contains(Actor))
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

void ASHProjectileBase::BindHomingTargetEvents(AActor* Target)
{
	if (Target)
	{
		Target->OnDestroyed.AddDynamic(this, &ASHProjectileBase::OnHomingTargetDestroyed);
	}
}

void ASHProjectileBase::UnbindHomingTargetEvents()
{
	if (HomingTarget.IsValid())
	{
		HomingTarget->OnDestroyed.RemoveDynamic(this, &ASHProjectileBase::OnHomingTargetDestroyed);
	}
}

void ASHProjectileBase::OnGlobalTimeScaleChanged(float NewTimeScale)
{
	if (ProjectileVFX)
	{
		ProjectileVFX->SetCustomTimeDilation(NewTimeScale);
	}
}
