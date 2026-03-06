// Sailing Heart

#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "AbilitySystem/SHGameplayEffectContext.h"
#include "SHGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

USHAttributeSetBase::USHAttributeSetBase()
{
	InitializeTagsToAttributes();
}

void USHAttributeSetBase::InitializeTagsToAttributes()
{
	const FSHGameplayTags& Tags = FSHGameplayTags::Get();

	TagsToAttributes.Add(Tags.Attribute_Vital_Health, GetHealthAttribute);
	TagsToAttributes.Add(Tags.Attribute_Vital_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(Tags.Attribute_Combat_Attack, GetAttackAttribute);
	TagsToAttributes.Add(Tags.Attribute_Combat_Defence, GetDefenceAttribute);
	TagsToAttributes.Add(Tags.Attribute_Combat_CriticalRate, GetCriticalRateAttribute);
	TagsToAttributes.Add(Tags.Attribute_Combat_CriticalDamage, GetCriticalDamageAttribute);
	TagsToAttributes.Add(Tags.Attribute_Meta_IncomingDamage, GetIncomingDamageAttribute);

	// 能量属性（现在是通用属性）
	TagsToAttributes.Add(Tags.Attribute_Player_Energy, GetEnergyAttribute);
	TagsToAttributes.Add(Tags.Attribute_Player_MaxEnergy, GetMaxEnergyAttribute);
	TagsToAttributes.Add(Tags.Attribute_Player_EnergyRegen, GetEnergyRegenAttribute);
}

void USHAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, Defence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, CriticalDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, Energy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, MaxEnergy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHAttributeSetBase, EnergyRegen, COND_None, REPNOTIFY_Always);
	// IncomingDamage 是元属性，不需要复制
}

FGameplayAttribute USHAttributeSetBase::GetAttributeByTag(const FGameplayTag& Tag) const
{
	if (const FAttributeFuncPtr* FuncPtr = TagsToAttributes.Find(Tag))
	{
		return (*FuncPtr)();
	}
	return FGameplayAttribute();
}

void USHAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 限制血量不超过最大值
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	// 限制防御力范围
	else if (Attribute == GetDefenceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 100.f);
	}
	// 限制暴击率范围
	else if (Attribute == GetCriticalRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 100.f);
	}
	// 限制暴击伤害最小值
	else if (Attribute == GetCriticalDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 100.f);  // 最少 100%
	}
	// 限制能量不超过最大值
	else if (Attribute == GetEnergyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxEnergy());
	}
}

void USHAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 处理接收的伤害
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);  // 重置元属性

		if (LocalIncomingDamage > 0.f)
		{
			// 扣除血量
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

			// 获取 Context 信息用于日志
			const FGameplayEffectContextHandle& ContextHandle = Data.EffectSpec.GetContext();
			const FSHGameplayEffectContext* SHContext = static_cast<const FSHGameplayEffectContext*>(ContextHandle.Get());

			AActor* TargetActor = nullptr;
			if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
			{
				TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
			}

			// 从 Context 获取伤害类型标签
			FString DamageTypeName = TEXT("Unknown");
			bool bIsCritical = false;
			if (SHContext)
			{
				bIsCritical = SHContext->IsCriticalHit();
				const FGameplayTag& DamageTag = SHContext->GetDamageTypeTag();
				if (DamageTag.IsValid())
				{
					DamageTypeName = DamageTag.GetTagName().ToString();
					int32 LastDotIndex;
					if (DamageTypeName.FindLastChar('.', LastDotIndex))
					{
						DamageTypeName = DamageTypeName.RightChop(LastDotIndex + 1);
					}
				}
			}

			// 检查是否死亡
			if (NewHealth <= 0.f)
			{
				// 广播死亡事件
				OnDeath.Broadcast(TargetActor);
			}
		}
	}

	// 限制血量
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void USHAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, Health, OldValue);
}

void USHAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, MaxHealth, OldValue);
}

void USHAttributeSetBase::OnRep_Attack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, Attack, OldValue);
}

void USHAttributeSetBase::OnRep_Defence(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, Defence, OldValue);
}

void USHAttributeSetBase::OnRep_CriticalRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, CriticalRate, OldValue);
}

void USHAttributeSetBase::OnRep_CriticalDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, CriticalDamage, OldValue);
}

void USHAttributeSetBase::OnRep_Energy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, Energy, OldValue);
}

void USHAttributeSetBase::OnRep_MaxEnergy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, MaxEnergy, OldValue);
}

void USHAttributeSetBase::OnRep_EnergyRegen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHAttributeSetBase, EnergyRegen, OldValue);
}
