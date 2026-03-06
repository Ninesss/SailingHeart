// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "SHGameplayEffectContext.generated.h"

/**
 * 自定义 GameplayEffectContext
 * 携带额外的伤害信息，使用 GameplayTagContainer 支持多伤害类型
 */
USTRUCT(BlueprintType)
struct SAILINGHEART_API FSHGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	FSHGameplayEffectContext()
		: FGameplayEffectContext()
		, bIsCriticalHit(false)
		, CollisionDamageOverride(0.f)
	{
	}

	// ========== 伤害类型 (支持多个 Tag) ==========

	const FGameplayTagContainer& GetDamageTypeTags() const { return DamageTypeTags; }
	void SetDamageTypeTags(const FGameplayTagContainer& InTags) { DamageTypeTags = InTags; }
	void AddDamageTypeTag(const FGameplayTag& InTag) { DamageTypeTags.AddTag(InTag); }

	// 向后兼容：单 Tag 接口
	FGameplayTag GetDamageTypeTag() const { return DamageTypeTags.First(); }
	void SetDamageTypeTag(const FGameplayTag& InTag) { DamageTypeTags.Reset(); DamageTypeTags.AddTag(InTag); }

	// 便捷方法：检查是否包含特定伤害类型
	bool HasDamageType(const FGameplayTag& InTag) const { return DamageTypeTags.HasTag(InTag); }

	// 便捷方法：检查是否是碰撞伤害
	bool IsCollisionDamage() const;

	// ========== 暴击 ==========

	bool IsCriticalHit() const { return bIsCriticalHit; }
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }

	// ========== 碰撞伤害覆盖值 ==========
	// 用于碰撞伤害时传递对方的剩余血量

	float GetCollisionDamageOverride() const { return CollisionDamageOverride; }
	void SetCollisionDamageOverride(float InDamage) { CollisionDamageOverride = InDamage; }

	// ========== 必须覆盖的方法 ==========

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FSHGameplayEffectContext::StaticStruct();
	}

	virtual FSHGameplayEffectContext* Duplicate() const override
	{
		FSHGameplayEffectContext* NewContext = new FSHGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	// 伤害类型标签（支持多个）
	UPROPERTY()
	FGameplayTagContainer DamageTypeTags;

	// 是否暴击
	UPROPERTY()
	bool bIsCriticalHit;

	// 碰撞伤害覆盖值（对方剩余血量）
	UPROPERTY()
	float CollisionDamageOverride;
};

// 启用网络序列化和复制
template<>
struct TStructOpsTypeTraits<FSHGameplayEffectContext> : TStructOpsTypeTraitsBase2<FSHGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
