// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 游戏中所有 Native GameplayTag 的集中定义
 * 使用单例模式，在 AssetManager 中初始化
 */
struct FSHGameplayTags
{
public:
	static const FSHGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	// ========== 伤害类型 ==========

	// 物理伤害（使用攻击/防御计算）
	FGameplayTag DamageType_Physical;

	// 碰撞伤害（使用剩余血量）
	FGameplayTag DamageType_Collision;

	// ========== 阵营标签 ==========

	FGameplayTag Faction_Player;
	FGameplayTag Faction_Enemy;
	FGameplayTag Faction_Neutral;

	// ========== 基础属性标签（共用）==========

	// 生命
	FGameplayTag Attribute_Vital_Health;
	FGameplayTag Attribute_Vital_MaxHealth;

	// 战斗
	FGameplayTag Attribute_Combat_Attack;
	FGameplayTag Attribute_Combat_Defence;
	FGameplayTag Attribute_Combat_CriticalRate;
	FGameplayTag Attribute_Combat_CriticalDamage;

	// 元属性
	FGameplayTag Attribute_Meta_IncomingDamage;

	// ========== 方块属性标签 ==========

	FGameplayTag Attribute_Block_Level;
	FGameplayTag Attribute_Block_ActionSpeed;

	// ========== 玩家属性标签 ==========

	FGameplayTag Attribute_Player_Energy;
	FGameplayTag Attribute_Player_MaxEnergy;
	FGameplayTag Attribute_Player_EnergyRegen;
	FGameplayTag Attribute_Player_MovementSpeed;

	// ========== Toggle激活状态标签 ==========

	FGameplayTag ToggleState_Player_TimeSlow;  // TimeSlow 激活状态

	// ========== 能力冷却标签 ==========

	FGameplayTag Cooldown_Player_Projectile_Linear;    // 基础投射物冷却
	FGameplayTag Cooldown_Player_Projectile_Homing;  // 追踪投射物冷却
	FGameplayTag Cooldown_Player_TimeSlow;           // 时间减缓冷却

	// ========== SetByCaller 标签 ==========

	FGameplayTag SetByCaller_Damage_Multiplier;  // 伤害倍率

private:
	static FSHGameplayTags GameplayTags;
};
