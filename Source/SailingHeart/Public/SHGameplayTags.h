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
	FGameplayTag Faction_Ally;    // 友军（ASHPlayerBlock 等玩家放置的单位）
	FGameplayTag Faction_Enemy;
	FGameplayTag Faction_Neutral;

	// ========== 战斗 Socket 标签 ==========

	FGameplayTag CombatSocket_Weapon;       // 通用武器/发射点 socket
	FGameplayTag CombatSocket_LeftHand;     // 左手近战 socket
	FGameplayTag CombatSocket_RightHand;    // 右手近战 socket
	FGameplayTag CombatSocket_AttackScene;  // 场景式攻击中心点

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

	// ========== 技能触发标签 ==========
	// DataAsset 的 TriggerTag 字段从这里选，StateTree 发送此 Tag 激活对应技能

	// 通用攻击槽：StateTree 只需填此 Tag，升级后自动触发新技能
	FGameplayTag AbilityTrigger_Block_Attack;        // 方块通用攻击槽

	FGameplayTag AbilityTrigger_Projectile_Linear;   // 直线投射物
	FGameplayTag AbilityTrigger_Projectile_Homing;   // 追踪投射物
	FGameplayTag AbilityTrigger_Melee_Basic;         // 基础近战
	FGameplayTag AbilityTrigger_TimeSlow;            // 时间减缓

	// ========== Toggle激活状态标签 ==========

	FGameplayTag ToggleState_Player_TimeSlow;  // TimeSlow 激活状态

	// ========== 能力冷却标签 ==========

	FGameplayTag Cooldown_Player_Projectile_Linear;   // 玩家直线投射物冷却
	FGameplayTag Cooldown_Player_Projectile_Homing;  // 玩家追踪投射物冷却
	FGameplayTag Cooldown_Player_TimeSlow;           // 时间减缓冷却

	FGameplayTag Cooldown_PBlock_Projectile_Linear;   // 方块直线投射物冷却
	FGameplayTag Cooldown_PBlock_Projectile_Homing;   // 方块追踪投射物冷却

	// ========== SetByCaller 标签 ==========

	FGameplayTag SetByCaller_Damage_Multiplier;  // 伤害倍率

private:
	static FSHGameplayTags GameplayTags;
};
