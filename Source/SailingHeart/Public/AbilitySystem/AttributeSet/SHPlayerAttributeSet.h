// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "SHPlayerAttributeSet.generated.h"

/**
 * 玩家属性集 - 继承基类，添加玩家特有属性
 */
UCLASS()
class SAILINGHEART_API USHPlayerAttributeSet : public USHAttributeSetBase
{
	GENERATED_BODY()

public:
	USHPlayerAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ========== 移动属性 ==========

	// 移动速度（百分比，100 = 正常速度）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Attributes|Movement")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(USHPlayerAttributeSet, MovementSpeed)

protected:
	virtual void InitializeTagsToAttributes() override;

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldValue);
};
