// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SHProjectileBase.generated.h"

class USphereComponent;
class USHProjectileMovementComponent;
class UNiagaraComponent;
class UGameplayEffect;
class UNiagaraSystem;
class UAudioComponent;
class ASHGameStateBase;

/**
 * 投射物命中效果配置
 */
USTRUCT(BlueprintType)
struct FSHProjectileHitEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float Level = 1.f;
};

/**
 * 投射物基类 - 直线飞行
 */
UCLASS(Abstract)
class SAILINGHEART_API ASHProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ASHProjectileBase();

	// ========== 组件 ==========

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USHProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ProjectileVFX;

	// ========== 伤害效果 ==========

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY(BlueprintReadWrite)
	TArray<FGameplayEffectSpecHandle> HitEffectSpecHandles;

	// ========== 配置 ==========

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float LifeSpan = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	bool bPenetrating = false;

	/** 最大穿透次数，0 = 无限穿透（只受 LifeSpan 限制） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bPenetrating", ClampMin = "0"))
	int32 MaxPenetrations = 0;

	/** Homing 只在 XY 平面追踪，不改变 Z */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	bool bHomingXYOnly = true;

	/** 目标死亡后自动寻找新目标 */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|Homing")
	bool bAutoRetarget = true;

	/** 重新寻敌范围（通过 Ability 生成时会被 DataAsset 覆盖） */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|Homing")
	float RetargetSearchRange = 2000.f;

	/** 设置 Homing 目标（会绑定目标销毁事件） */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetHomingTarget(AActor* NewTarget);

	// ========== 视觉/音效 ==========

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|FX")
	TObjectPtr<UNiagaraSystem> SpawnEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|FX")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Sound")
	TObjectPtr<USoundBase> SpawnSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Sound")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Sound")
	TObjectPtr<USoundBase> LoopingSound;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayImpactFX(const FVector& Location, const FRotator& Rotation);

	void PlaySpawnFX();

	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	virtual void HandleHitTarget(AActor* HitActor, const FHitResult& HitResult);
	void ApplyDamageToTarget(AActor* TargetActor);
	void ApplyHitEffectsToTarget(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void OnProjectileDestroyed();
	virtual void OnProjectileDestroyed_Implementation();

	bool bHit = false;
	int32 PenetrationCount = 0;

	UPROPERTY()
	TSet<AActor*> HitActors;

	// ========== Homing 重寻敌 ==========

	UPROPERTY(ReplicatedUsing = OnRep_HomingTarget)
	TWeakObjectPtr<AActor> HomingTarget;

	UFUNCTION()
	void OnRep_HomingTarget();

	UFUNCTION()
	void OnHomingTargetDestroyed(AActor* DestroyedActor);

	AActor* FindNearestEnemy(AActor* ExcludeActor = nullptr) const;

	void BindHomingTargetEvents(AActor* Target);
	void UnbindHomingTargetEvents();

	// ========== GlobalTimeScale 事件 ==========

	TWeakObjectPtr<ASHGameStateBase> CachedGameState;

	UFUNCTION()
	void OnGlobalTimeScaleChanged(float NewTimeScale);
};
