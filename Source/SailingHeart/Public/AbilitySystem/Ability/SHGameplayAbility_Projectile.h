// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/SHGameplayAbilityBase.h"
#include "AbilitySystem/Ability/SHDamageAbilityInterface.h"
#include "Projectile/SHProjectileBase.h"
#include "Data/Ability/SHAbilityParams.h"
#include "GameplayTagContainer.h"
#include "SHGameplayAbility_Projectile.generated.h"

class UGameplayEffect;
class USHProjectileAbilityData;
class UAbilityTask_WaitGameplayEvent;
struct FSHProjectileAbilityParams;

/**
 * 投射物技能 - 支持多种移动类型和可选目标
 * 实现 ISHDamageAbilityInterface 接口
 *
 * Target 获取优先级：
 * 1. 从 TriggerEventData 获取（StateTree 传入）
 * 2. 自动寻找最近敌人（TargetSearchRange > 0 时）
 * 3. 无目标则朝前方发射
 *
 * 移动类型：
 * - Linear：直线飞行
 * - Homing：追踪目标
 * - Parabolic：抛物线飞行（未实现）
 *
 * 数据来源：USHProjectileAbilityData
 */
UCLASS(Abstract)
class SAILINGHEART_API USHGameplayAbility_Projectile
	: public USHGameplayAbilityBase
	, public ISHDamageAbilityInterface
{
	GENERATED_BODY()

public:
	USHGameplayAbility_Projectile();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// ========== ISHDamageAbilityInterface 实现 ==========

	virtual TSubclassOf<UGameplayEffect> GetDamageEffectClass_Implementation() const override;
	virtual FGameplayTagContainer GetDamageTypeTags_Implementation() const override;
	virtual float GetDamageMultiplier_Implementation() const override;
	virtual UAbilitySystemComponent* GetDamageSourceASC_Implementation() const override;
	virtual AActor* GetDamageSourceActor_Implementation() const override;

	// ========== DataAsset 获取 ==========

	// 获取当前等级的参数（子类可重写使用自己的 DataAsset 类型）
	virtual const FSHProjectileAbilityParams* GetCurrentParams() const;

	// ========== 参数获取（从 DataAsset 读取）==========

	TSubclassOf<UGameplayEffect> GetDamageEffectClassParam() const;
	FGameplayTagContainer GetDamageTypeTagsParam() const;
	float GetDamageMultiplierParam() const;
	TSubclassOf<ASHProjectileBase> GetProjectileClassParam() const;
	float GetProjectileSpeedParam() const;
	float GetLifeSpanParam() const;
	FVector GetSpawnOffsetParam() const;
	int32 GetProjectilesPerActivationParam() const;
	float GetSpreadAngleParam() const;
	bool GetPenetratingParam() const;
	int32 GetMaxPenetrationsParam() const;
	TArray<FSHProjectileHitEffect> GetHitEffectsParam() const;

	// 目标和移动类型参数
	float GetTargetSearchRangeParam() const;
	float GetRetargetSearchRangeParam() const;
	EProjectileMovementType GetMovementTypeParam() const;
	float GetHomingAccelerationParam() const;
	bool GetHomingXYOnlyParam() const;
	bool GetAutoRetargetParam() const;

	// ========== Cost/Cooldown 获取 ==========

	virtual float GetEnergyCost() const override;
	virtual float GetAbilityCooldownDuration() const override;

	// ========== 目标获取 ==========

	// 获取目标（优先 Event，否则自动寻敌）
	AActor* AcquireTarget(const FGameplayEventData* TriggerEventData);

	// 自动寻找最近敌人
	AActor* FindNearestEnemy(const FVector& Origin) const;

	// 当前目标（可能为空，在 ActivateAbility 时缓存）
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	// ========== 动画 + Event 触发 ==========

	/**
	 * 播放攻击 Montage 并注册 WaitGameplayEvent
	 * Montage 中的 USHAbilityAnimNotify 会在关键帧时发送 EventTag 事件
	 * 事件触发后调用 OnAnimNotifyEvent，执行实际发射逻辑
	 */
	void PlayMontageAndWaitForEvent(const FGameplayAbilityActorInfo* ActorInfo);

	// AnimNotify 触发时的回调
	UFUNCTION()
	void OnAnimNotifyEvent(FGameplayEventData Payload);

	// ========== 生成逻辑 ==========

	virtual FVector GetSpawnLocation(const FGameplayAbilityActorInfo* ActorInfo) const;
	virtual FRotator GetSpawnRotation(const FGameplayAbilityActorInfo* ActorInfo, int32 ProjectileIndex) const;

	/**
	 * 通过 Socket 位置生成旋转（AnimNotify 触发时使用）
	 * @param SocketLocation  Notify 传入的 Socket 世界位置
	 * @param ProjectileIndex 当前投射物序号（用于多发扩散计算）
	 */
	FRotator GetSpawnRotationFromSocket(const FVector& SocketLocation, int32 ProjectileIndex) const;

	virtual ASHProjectileBase* SpawnProjectile(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& Location,
		const FRotator& Rotation);

	/** 配置投射物，在 FinishSpawning 前调用，子类重写添加额外配置 */
	virtual void ConfigureProjectile(ASHProjectileBase* Projectile, const FGameplayAbilityActorInfo* ActorInfo);

	void FireProjectiles(const FGameplayAbilityActorInfo* ActorInfo);

	/**
	 * 从 AnimNotify Payload 提取 Socket 位置后发射投射物
	 * @param SocketLocation  Notify 中 FunctionalSKM Socket 的世界位置
	 */
	void FireProjectilesFromSocket(const FVector& SocketLocation);
};
