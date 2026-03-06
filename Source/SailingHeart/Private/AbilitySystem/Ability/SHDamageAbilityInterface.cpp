// Sailing Heart

#include "AbilitySystem/Ability/SHDamageAbilityInterface.h"
#include "AbilitySystem/SHAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "SHGameplayTags.h"

FGameplayEffectSpecHandle ISHDamageAbilityInterface::MakeDamageSpec_Implementation(float Level) const
{
	// 调用接口函数获取参数
	UObject* ThisObject = const_cast<UObject*>(Cast<UObject>(this));
	TSubclassOf<UGameplayEffect> DamageGE = Execute_GetDamageEffectClass(ThisObject);
	FGameplayTagContainer DamageTypes = Execute_GetDamageTypeTags(ThisObject);
	UAbilitySystemComponent* SourceASC = Execute_GetDamageSourceASC(ThisObject);
	AActor* SourceActor = Execute_GetDamageSourceActor(ThisObject);
	float DamageMultiplier = Execute_GetDamageMultiplier(ThisObject);

	if (!DamageGE || !SourceASC)
	{
		return FGameplayEffectSpecHandle();
	}

	// 创建 Context
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	if (SourceActor)
	{
		ContextHandle.AddSourceObject(SourceActor);
	}

	// 设置伤害类型（如果未配置，默认使用物理伤害）
	if (DamageTypes.IsEmpty())
	{
		USHAbilitySystemLibrary::SetDamageTypeTag(ContextHandle, FSHGameplayTags::Get().DamageType_Physical);
	}
	else
	{
		USHAbilitySystemLibrary::SetDamageTypeTags(ContextHandle, DamageTypes);
	}

	// 创建 Spec
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGE, Level, ContextHandle);

	// 设置伤害倍率（SetByCaller）
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FSHGameplayTags::Get().SetByCaller_Damage_Multiplier, DamageMultiplier);
	}

	return SpecHandle;
}

bool ISHDamageAbilityInterface::ApplyDamageToTarget_Implementation(AActor* Target, float Level)
{
	if (!Target)
	{
		return false;
	}

	UAbilitySystemComponent* SourceASC = Execute_GetDamageSourceASC(Cast<UObject>(this));
	if (!SourceASC)
	{
		return false;
	}

	// 获取目标 ASC
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (!TargetASC)
	{
		return false;
	}

	// 创建并应用伤害
	FGameplayEffectSpecHandle SpecHandle = Execute_MakeDamageSpec(Cast<UObject>(this), Level);
	if (SpecHandle.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		return true;
	}

	return false;
}

int32 ISHDamageAbilityInterface::ApplyDamageToTargets_Implementation(const TArray<AActor*>& Targets, float Level)
{
	UAbilitySystemComponent* SourceASC = Execute_GetDamageSourceASC(Cast<UObject>(this));
	if (!SourceASC)
	{
		return 0;
	}

	// 创建一次 Spec，复用给所有目标
	FGameplayEffectSpecHandle SpecHandle = Execute_MakeDamageSpec(Cast<UObject>(this), Level);
	if (!SpecHandle.IsValid())
	{
		return 0;
	}

	int32 SuccessCount = 0;
	for (AActor* Target : Targets)
	{
		if (!Target)
		{
			continue;
		}

		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
		UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
		if (TargetASC)
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			++SuccessCount;
		}
	}

	return SuccessCount;
}
