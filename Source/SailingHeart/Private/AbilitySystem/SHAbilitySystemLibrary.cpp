// Sailing Heart

#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "AbilitySystem/SHGameplayEffectContext.h"
#include "SHGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"

const FSHGameplayEffectContext* USHAbilitySystemLibrary::GetSHEffectContextConst(const FGameplayEffectContextHandle& ContextHandle)
{
	const FGameplayEffectContext* Context = ContextHandle.Get();
	if (!Context)
	{
		return nullptr;
	}

	// 检查是否是正确的类型
	if (Context->GetScriptStruct() != FSHGameplayEffectContext::StaticStruct())
	{
		return nullptr;
	}

	return static_cast<const FSHGameplayEffectContext*>(Context);
}

FSHGameplayEffectContext* USHAbilitySystemLibrary::GetSHEffectContext(FGameplayEffectContextHandle& ContextHandle)
{
	const FGameplayEffectContext* Context = ContextHandle.Get();
	if (!Context)
	{
		return nullptr;
	}

	// 检查是否是正确的类型
	if (Context->GetScriptStruct() != FSHGameplayEffectContext::StaticStruct())
	{
		return nullptr;
	}

	// 使用 const_cast 因为 Handle 内部的 Context 是可修改的
	return const_cast<FSHGameplayEffectContext*>(static_cast<const FSHGameplayEffectContext*>(Context));
}

void USHAbilitySystemLibrary::SetDamageTypeTags(FGameplayEffectContextHandle& ContextHandle, const FGameplayTagContainer& DamageTypeTags)
{
	if (FSHGameplayEffectContext* SHContext = GetSHEffectContext(ContextHandle))
	{
		SHContext->SetDamageTypeTags(DamageTypeTags);
	}
}

FGameplayTagContainer USHAbilitySystemLibrary::GetDamageTypeTags(const FGameplayEffectContextHandle& ContextHandle)
{
	if (const FSHGameplayEffectContext* SHContext = GetSHEffectContextConst(ContextHandle))
	{
		return SHContext->GetDamageTypeTags();
	}
	return FGameplayTagContainer();
}

void USHAbilitySystemLibrary::AddDamageTypeTag(FGameplayEffectContextHandle& ContextHandle, const FGameplayTag& DamageTypeTag)
{
	if (FSHGameplayEffectContext* SHContext = GetSHEffectContext(ContextHandle))
	{
		SHContext->AddDamageTypeTag(DamageTypeTag);
	}
}

void USHAbilitySystemLibrary::SetDamageTypeTag(FGameplayEffectContextHandle& ContextHandle, const FGameplayTag& DamageTypeTag)
{
	if (FSHGameplayEffectContext* SHContext = GetSHEffectContext(ContextHandle))
	{
		SHContext->SetDamageTypeTag(DamageTypeTag);
	}
}

FGameplayTag USHAbilitySystemLibrary::GetDamageTypeTag(const FGameplayEffectContextHandle& ContextHandle)
{
	if (const FSHGameplayEffectContext* SHContext = GetSHEffectContextConst(ContextHandle))
	{
		return SHContext->GetDamageTypeTag();
	}
	return FGameplayTag();
}

bool USHAbilitySystemLibrary::IsCollisionDamage(const FGameplayEffectContextHandle& ContextHandle)
{
	if (const FSHGameplayEffectContext* SHContext = GetSHEffectContextConst(ContextHandle))
	{
		return SHContext->IsCollisionDamage();
	}
	return false;
}

void USHAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& ContextHandle, bool bIsCriticalHit)
{
	if (FSHGameplayEffectContext* SHContext = GetSHEffectContext(ContextHandle))
	{
		SHContext->SetIsCriticalHit(bIsCriticalHit);
	}
}

bool USHAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& ContextHandle)
{
	if (const FSHGameplayEffectContext* SHContext = GetSHEffectContextConst(ContextHandle))
	{
		return SHContext->IsCriticalHit();
	}
	return false;
}

void USHAbilitySystemLibrary::SetCollisionDamageOverride(FGameplayEffectContextHandle& ContextHandle, float Damage)
{
	if (FSHGameplayEffectContext* SHContext = GetSHEffectContext(ContextHandle))
	{
		SHContext->SetCollisionDamageOverride(Damage);
	}
}

FGameplayTag USHAbilitySystemLibrary::GetActorFaction(AActor* Actor)
{
	if (!Actor)
	{
		return FGameplayTag();
	}

	// 尝试获取 ASC
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const FSHGameplayTags& Tags = FSHGameplayTags::Get();

			// 检查各阵营标签
			if (ASC->HasMatchingGameplayTag(Tags.Faction_Player))
			{
				return Tags.Faction_Player;
			}
			if (ASC->HasMatchingGameplayTag(Tags.Faction_Enemy))
			{
				return Tags.Faction_Enemy;
			}
			if (ASC->HasMatchingGameplayTag(Tags.Faction_Neutral))
			{
				return Tags.Faction_Neutral;
			}
		}
	}

	return FGameplayTag();
}

bool USHAbilitySystemLibrary::AreActorsFriends(AActor* FirstActor, AActor* SecondActor)
{
	if (!FirstActor || !SecondActor)
	{
		return false;
	}

	const FGameplayTag F1 = GetActorFaction(FirstActor);
	const FGameplayTag F2 = GetActorFaction(SecondActor);

	if (!F1.IsValid() || !F2.IsValid())
	{
		return false;
	}

	const FSHGameplayTags& Tags = FSHGameplayTags::Get();

	// 中立方不是任何人的朋友
	if (F1 == Tags.Faction_Neutral || F2 == Tags.Faction_Neutral)
	{
		return false;
	}

	// 同阵营为友方
	return F1 == F2;
}

bool USHAbilitySystemLibrary::AreActorsEnemies(AActor* FirstActor, AActor* SecondActor)
{
	if (!FirstActor || !SecondActor)
	{
		return false;
	}

	const FGameplayTag F1 = GetActorFaction(FirstActor);
	const FGameplayTag F2 = GetActorFaction(SecondActor);

	if (!F1.IsValid() || !F2.IsValid())
	{
		return false;
	}

	const FSHGameplayTags& Tags = FSHGameplayTags::Get();

	// 中立方不是敌人
	if (F1 == Tags.Faction_Neutral || F2 == Tags.Faction_Neutral)
	{
		return false;
	}

	// Player vs Enemy 为敌方
	return F1 != F2;
}

void USHAbilitySystemLibrary::ApplyDamage(
	UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> DamageEffectClass)
{
	if (!SourceASC || !TargetASC || !DamageEffectClass)
	{
		return;
	}

	// 创建 Effect Context
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(SourceASC->GetAvatarActor());

	// 设置为物理伤害（暴击在 ExecCalc 中判定）
	SetDamageTypeTag(ContextHandle, FSHGameplayTags::Get().DamageType_Physical);

	// 创建 Effect Spec
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ContextHandle);
	if (SpecHandle.IsValid())
	{
		// 应用到目标
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void USHAbilitySystemLibrary::ApplyCollisionDamage(
	UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	float DamageAmount)
{
	if (!SourceASC || !TargetASC || !DamageEffectClass)
	{
		return;
	}

	// 创建 Effect Context
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(SourceASC->GetAvatarActor());

	// 设置为碰撞伤害并存储伤害值
	SetDamageTypeTag(ContextHandle, FSHGameplayTags::Get().DamageType_Collision);
	SetCollisionDamageOverride(ContextHandle, DamageAmount);

	// 创建 Effect Spec
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ContextHandle);
	if (SpecHandle.IsValid())
	{
		// 应用到目标
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void USHAbilitySystemLibrary::ApplyBuffDebuff(
	UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> EffectClass,
	float Level)
{
	if (!SourceASC || !TargetASC || !EffectClass)
	{
		return;
	}

	// 创建 Effect Context
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(SourceASC->GetAvatarActor());

	// 创建 Effect Spec
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, ContextHandle);
	if (SpecHandle.IsValid())
	{
		// 应用到目标
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}
