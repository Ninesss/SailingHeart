// Sailing Heart

#include "AbilitySystem/AttributeSet/SHBlockAttributeSet.h"
#include "SHGameplayTags.h"
#include "Net/UnrealNetwork.h"

USHBlockAttributeSet::USHBlockAttributeSet()
{
}

void USHBlockAttributeSet::InitializeTagsToAttributes()
{
	// 先调用基类初始化共用属性
	Super::InitializeTagsToAttributes();

	// 添加方块特有属性映射
	const FSHGameplayTags& Tags = FSHGameplayTags::Get();
	TagsToAttributes.Add(Tags.Attribute_Block_Level, GetLevelAttribute);
	TagsToAttributes.Add(Tags.Attribute_Block_ActionSpeed, GetActionSpeedAttribute);
}

void USHBlockAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USHBlockAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USHBlockAttributeSet, ActionSpeed, COND_None, REPNOTIFY_Always);
}

void USHBlockAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 限制等级最小值
	if (Attribute == GetLevelAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	// 限制行动速度最小值
	else if (Attribute == GetActionSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);  // 最少 1%
	}
}

void USHBlockAttributeSet::OnRep_Level(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHBlockAttributeSet, Level, OldValue);
}

void USHBlockAttributeSet::OnRep_ActionSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHBlockAttributeSet, ActionSpeed, OldValue);
}
