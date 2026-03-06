// Sailing Heart

#include "AbilitySystem/AttributeSet/SHPlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "SHGameplayTags.h"
#include "Net/UnrealNetwork.h"

USHPlayerAttributeSet::USHPlayerAttributeSet()
{
}

void USHPlayerAttributeSet::InitializeTagsToAttributes()
{
	// 先调用基类初始化共用属性（包括 Energy）
	Super::InitializeTagsToAttributes();

	// 添加玩家特有属性映射
	const FSHGameplayTags& Tags = FSHGameplayTags::Get();
	TagsToAttributes.Add(Tags.Attribute_Player_MovementSpeed, GetMovementSpeedAttribute);
}

void USHPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USHPlayerAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void USHPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 限制移动速度最小值
	if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);  // 最少 1%
	}
}

void USHPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 能量变化由各 Toggle 能力自己监听处理
	// 不在 AttributeSet 中硬编码具体能力的逻辑
}

void USHPlayerAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USHPlayerAttributeSet, MovementSpeed, OldValue);
}
