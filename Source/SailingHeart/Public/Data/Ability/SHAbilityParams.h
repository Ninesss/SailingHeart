// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Projectile/SHProjectileBase.h"
#include "SHAbilityParams.generated.h"

class UGameplayEffect;

/**
 * 能力参数基类
 * 所有能力参数结构体都应继承此类
 */
USTRUCT(BlueprintType)
struct SAILINGHEART_API FSHAbilityParams
{
	GENERATED_BODY()

	virtual ~FSHAbilityParams() = default;

	// ========== Cost配置 ==========

	// 激活时一次性消耗的能量（0 = 无消耗）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float EnergyCost = 0.f;

	// ========== Cooldown配置 ==========

	// 冷却时间（秒，0 = 无冷却）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown")
	float CooldownDuration = 0.f;

	// 冷却Tag（在编辑器中选择，如 Cooldown.Ability.Projectile）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;
};

/**
 * Toggle 能力参数
 * 注意：CooldownDuration 继承自基类 FSHAbilityParams
 * 关闭时必定进入冷却（无论手动关闭还是能量不足自动关闭）
 */
USTRUCT(BlueprintType)
struct SAILINGHEART_API FSHToggleAbilityParams : public FSHAbilityParams
{
	GENERATED_BODY()

	FSHToggleAbilityParams()
	{
		// Toggle默认有0.5秒冷却
		CooldownDuration = 0.5f;
	}

	// 激活状态Tag（在编辑器中选择，如 ToggleState.CPA.TimeSlow）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle", meta = (Categories = "ToggleState"))
	FGameplayTag ActiveTag;

	// 每秒能量消耗（0 = 无持续消耗）
	// 当 Energy < EnergyCostPerSecond 时自动关闭
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle|Cost")
	float EnergyCostPerSecond = 0.f;
};

/**
 * 时间减缓能力参数
 * 注意：SlowFactor 由 GameState 硬编码控制（SlowedTimeScale = 0.1f）
 * EnergyCostPerSecond 继承自 FSHToggleAbilityParams
 */
USTRUCT(BlueprintType)
struct SAILINGHEART_API FSHTimeSlowAbilityParams : public FSHToggleAbilityParams
{
	GENERATED_BODY()

	// 时间减缓特有参数可在此添加
};

/**
 * 投射物移动类型
 */
UENUM(BlueprintType)
enum class EProjectileMovementType : uint8
{
	// 直线飞行（默认）
	Linear,
	// 追踪目标
	Homing,
	// 抛物线飞行（未实现）
	Parabolic,
};

/**
 * 投射物能力参数
 *
 * Target 获取优先级：
 * 1. 从 TriggerEventData 获取（StateTree 传入）
 * 2. 自动寻找最近敌人（TargetSearchRange > 0 时）
 * 3. 无目标则朝前方发射
 */
USTRUCT(BlueprintType)
struct SAILINGHEART_API FSHProjectileAbilityParams : public FSHAbilityParams
{
	GENERATED_BODY()

	// ========== 伤害配置 ==========

	// 伤害 GE 类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 伤害类型标签
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Categories = "DamageType"))
	FGameplayTagContainer DamageTypeTags;

	// 伤害倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	// ========== 投射物配置 ==========

	// 投射物类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<ASHProjectileBase> ProjectileClass;

	// 投射物速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileSpeed = 2000.f;

	// 投射物存活时间（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0.1"))
	float LifeSpan = 10.f;

	// 生成偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector SpawnOffset = FVector(100.f, 0.f, 0.f);

	// 每次激活发射数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|MultiShot")
	int32 ProjectilesPerActivation = 1;

	// 扩散角度（多发时）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|MultiShot")
	float SpreadAngle = 15.f;

	// ========== 穿透配置 ==========

	// 是否穿透
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Penetration")
	bool bPenetrating = false;

	// 最大穿透次数（0 = 无限）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Penetration")
	int32 MaxPenetrations = 0;

	// ========== 命中效果 ==========

	// 命中时施加的额外效果（如减速、Debuff 等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	TArray<FSHProjectileHitEffect> HitEffects;

	// ========== 目标配置（可选）==========

	// 自动寻敌范围（0 = 不自动寻敌）
	// 优先使用 Event 传入的 Target，没有时才自动寻敌
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	float TargetSearchRange = 0.f;

	// 只在 XY 平面朝目标（不改变 Z 高度）
	// 对 Linear 影响初始发射方向，对 Homing 影响追踪方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	bool bHomingXYOnly = true;

	// ========== 移动类型 ==========

	// 投射物移动方式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EProjectileMovementType MovementType = EProjectileMovementType::Linear;

	// ========== Homing 参数（MovementType == Homing 时使用）==========

	// 追踪加速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Homing", meta = (EditCondition = "MovementType == EProjectileMovementType::Homing", EditConditionHides))
	float HomingAcceleration = 5000.f;

	// 目标死亡后自动寻找新目标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Homing", meta = (EditCondition = "MovementType == EProjectileMovementType::Homing", EditConditionHides))
	bool bAutoRetarget = true;

	// 重新寻敌范围（目标死亡后搜索新目标的范围，0 = 使用 TargetSearchRange）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Homing", meta = (EditCondition = "MovementType == EProjectileMovementType::Homing && bAutoRetarget", EditConditionHides))
	float RetargetSearchRange = 0.f;

	// ========== Parabolic 参数（MovementType == Parabolic 时使用，未实现）==========

	// 抛物线最高点高度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Parabolic", meta = (EditCondition = "MovementType == EProjectileMovementType::Parabolic", EditConditionHides))
	float ParabolicArcHeight = 500.f;
};
