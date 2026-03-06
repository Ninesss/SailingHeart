// Sailing Heart

#include "SHGameplayTags.h"
#include "GameplayTagsManager.h"

FSHGameplayTags FSHGameplayTags::GameplayTags;

void FSHGameplayTags::InitializeNativeGameplayTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// ========== 伤害类型 ==========

	GameplayTags.DamageType_Physical = Manager.AddNativeGameplayTag(
		FName("DamageType.Physical"),
		FString("Physical damage using Attack/Defence calculation")
	);

	GameplayTags.DamageType_Collision = Manager.AddNativeGameplayTag(
		FName("DamageType.Collision"),
		FString("Collision damage using remaining health")
	);

	// ========== 阵营标签 ==========

	GameplayTags.Faction_Player = Manager.AddNativeGameplayTag(
		FName("Faction.Player"),
		FString("Player faction")
	);

	GameplayTags.Faction_Enemy = Manager.AddNativeGameplayTag(
		FName("Faction.Enemy"),
		FString("Enemy faction")
	);

	GameplayTags.Faction_Neutral = Manager.AddNativeGameplayTag(
		FName("Faction.Neutral"),
		FString("Neutral faction (obstacles, etc.)")
	);

	// ========== 基础属性标签（共用）==========

	GameplayTags.Attribute_Vital_Health = Manager.AddNativeGameplayTag(
		FName("Attribute.Vital.Health"),
		FString("Current health")
	);

	GameplayTags.Attribute_Vital_MaxHealth = Manager.AddNativeGameplayTag(
		FName("Attribute.Vital.MaxHealth"),
		FString("Maximum health")
	);

	GameplayTags.Attribute_Combat_Attack = Manager.AddNativeGameplayTag(
		FName("Attribute.Combat.Attack"),
		FString("Attack power")
	);

	GameplayTags.Attribute_Combat_Defence = Manager.AddNativeGameplayTag(
		FName("Attribute.Combat.Defence"),
		FString("Defence value (percentage)")
	);

	GameplayTags.Attribute_Combat_CriticalRate = Manager.AddNativeGameplayTag(
		FName("Attribute.Combat.CriticalRate"),
		FString("Critical hit rate (percentage)")
	);

	GameplayTags.Attribute_Combat_CriticalDamage = Manager.AddNativeGameplayTag(
		FName("Attribute.Combat.CriticalDamage"),
		FString("Critical hit damage multiplier (percentage)")
	);

	GameplayTags.Attribute_Meta_IncomingDamage = Manager.AddNativeGameplayTag(
		FName("Attribute.Meta.IncomingDamage"),
		FString("Meta attribute for incoming damage calculation")
	);

	// ========== 方块属性标签 ==========

	GameplayTags.Attribute_Block_Level = Manager.AddNativeGameplayTag(
		FName("Attribute.Block.Level"),
		FString("Block level")
	);

	GameplayTags.Attribute_Block_ActionSpeed = Manager.AddNativeGameplayTag(
		FName("Attribute.Block.ActionSpeed"),
		FString("Block action speed (percentage)")
	);

	// ========== 玩家属性标签 ==========

	GameplayTags.Attribute_Player_Energy = Manager.AddNativeGameplayTag(
		FName("Attribute.Player.Energy"),
		FString("Current energy")
	);

	GameplayTags.Attribute_Player_MaxEnergy = Manager.AddNativeGameplayTag(
		FName("Attribute.Player.MaxEnergy"),
		FString("Maximum energy")
	);

	GameplayTags.Attribute_Player_EnergyRegen = Manager.AddNativeGameplayTag(
		FName("Attribute.Player.EnergyRegen"),
		FString("Energy regeneration per second")
	);

	GameplayTags.Attribute_Player_MovementSpeed = Manager.AddNativeGameplayTag(
		FName("Attribute.Player.MovementSpeed"),
		FString("Movement speed multiplier (percentage)")
	);

	// ========== Toggle激活状态标签 ==========

	GameplayTags.ToggleState_Player_TimeSlow = Manager.AddNativeGameplayTag(
		FName("ToggleState.Player.TimeSlow"),
		FString("Time slow ability active state")
	);

	// ========== 能力冷却标签 ==========

	GameplayTags.Cooldown_Player_Projectile_Linear = Manager.AddNativeGameplayTag(
		FName("Cooldown.Player.Projectile.Linear"),
		FString("Linear projectile ability cooldown")
	);

	GameplayTags.Cooldown_Player_Projectile_Homing = Manager.AddNativeGameplayTag(
		FName("Cooldown.Player.Projectile.Homing"),
		FString("Homing projectile ability cooldown")
	);

	GameplayTags.Cooldown_Player_TimeSlow = Manager.AddNativeGameplayTag(
		FName("Cooldown.Player.TimeSlow"),
		FString("Time slow ability cooldown")
	);

	// ========== SetByCaller 标签 ==========

	GameplayTags.SetByCaller_Damage_Multiplier = Manager.AddNativeGameplayTag(
		FName("SetByCaller.Damage.Multiplier"),
		FString("Damage multiplier for ability damage calculation")
	);
}
