// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "SHAttributeSetBase.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// 前向声明
struct FGameplayEffectModCallbackData;

// 委托：死亡时广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);

// 属性获取函数指针类型
typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;

/**
 * 属性集基类 - 包含玩家和方块共用的属性
 */
UCLASS()
class SAILINGHEART_API USHAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	USHAttributeSetBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// GameplayEffect 执行后的回调（处理伤害）
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 属性值变化前的限制
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// ========== Tag-to-Attribute 映射 ==========

	// 根据 GameplayTag 获取对应的属性
	FGameplayAttribute GetAttributeByTag(const FGameplayTag& Tag) const;

	// 获取所有映射（供外部使用）
	const TMap<FGameplayTag, FAttributeFuncPtr>& GetTagsToAttributes() const { return TagsToAttributes; }

	// ========== 生命属性 ==========

	// 当前血量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes|Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, Health)

	// 最大血量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes|Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, MaxHealth)

	// ========== 战斗属性 ==========

	// 攻击力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "Attributes|Combat")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, Attack)

	// 防御力（百分比，0-100）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defence, Category = "Attributes|Combat")
	FGameplayAttributeData Defence;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, Defence)

	// 暴击率（百分比，0-100）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalRate, Category = "Attributes|Combat")
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, CriticalRate)

	// 暴击伤害（百分比，如 150 表示 150% 伤害）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalDamage, Category = "Attributes|Combat")
	FGameplayAttributeData CriticalDamage;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, CriticalDamage)

	// ========== 能量属性 ==========

	// 当前能量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Attributes|Energy")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, Energy)

	// 最大能量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEnergy, Category = "Attributes|Energy")
	FGameplayAttributeData MaxEnergy;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, MaxEnergy)

	// 能量回复速度（每秒回复量）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EnergyRegen, Category = "Attributes|Energy")
	FGameplayAttributeData EnergyRegen;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, EnergyRegen)

	// ========== 元属性（不复制，仅用于计算）==========

	// 接收的伤害（由 ExecCalc 设置，在 PostGameplayEffectExecute 中处理）
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(USHAttributeSetBase, IncomingDamage)

	// ========== 委托 ==========

	// 死亡委托
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeath OnDeath;

protected:
	// 初始化 Tag-to-Attribute 映射（子类可覆盖添加更多映射）
	virtual void InitializeTagsToAttributes();

	// 属性复制通知
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defence(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_EnergyRegen(const FGameplayAttributeData& OldValue);

	// Tag 到属性的映射
	TMap<FGameplayTag, FAttributeFuncPtr> TagsToAttributes;
};
