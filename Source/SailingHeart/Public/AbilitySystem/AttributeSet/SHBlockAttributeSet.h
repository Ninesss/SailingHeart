// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "SHBlockAttributeSet.generated.h"

/**
 * 方块属性集 - 继承基类，添加方块特有属性
 */
UCLASS()
class SAILINGHEART_API USHBlockAttributeSet : public USHAttributeSetBase
{
	GENERATED_BODY()

public:
	USHBlockAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// ========== 方块特有属性 ==========

	// 等级
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Attributes|Block")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(USHBlockAttributeSet, Level)

	// 行动速度（影响技能触发频率，百分比，100 = 正常速度）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ActionSpeed, Category = "Attributes|Block")
	FGameplayAttributeData ActionSpeed;
	ATTRIBUTE_ACCESSORS(USHBlockAttributeSet, ActionSpeed)

protected:
	virtual void InitializeTagsToAttributes() override;

	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ActionSpeed(const FGameplayAttributeData& OldValue);
};
