// Sailing Heart

#include "AbilitySystem/SHGameplayEffectContext.h"
#include "SHGameplayTags.h"

bool FSHGameplayEffectContext::IsCollisionDamage() const
{
	return DamageTypeTags.HasTagExact(FSHGameplayTags::Get().DamageType_Collision);
}

bool FSHGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint32 RepBits = 0;

	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
			RepBits |= 1 << 0;
		if (bReplicateEffectCauser && EffectCauser.IsValid())
			RepBits |= 1 << 1;
		if (AbilityCDO.IsValid())
			RepBits |= 1 << 2;
		if (bReplicateSourceObject && SourceObject.IsValid())
			RepBits |= 1 << 3;
		if (Actors.Num() > 0)
			RepBits |= 1 << 4;
		if (HitResult.IsValid())
			RepBits |= 1 << 5;
		if (bHasWorldOrigin)
			RepBits |= 1 << 6;
		// 自定义字段
		if (!DamageTypeTags.IsEmpty())
			RepBits |= 1 << 7;
		if (bIsCriticalHit)
			RepBits |= 1 << 8;
		if (CollisionDamageOverride > 0.f)
			RepBits |= 1 << 9;
	}

	// 使用 16 位来容纳所有标志
	Ar.SerializeBits(&RepBits, 16);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	// 自定义字段序列化
	if (RepBits & (1 << 7))
	{
		// 序列化 GameplayTagContainer
		DamageTypeTags.NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}
	if (RepBits & (1 << 9))
	{
		Ar << CollisionDamageOverride;
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get());
	}

	bOutSuccess = true;
	return true;
}
