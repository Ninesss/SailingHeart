// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Character/SHCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "Interface/SHCombatInterface.h"
#include "SHCombatCharacterBase.generated.h"

class UAbilitySystemComponent;
class USHAttributeSetBase;

/**
 * 角色初始化参数
 */
USTRUCT(BlueprintType)
struct FCharacterInitParams
{
	GENERATED_BODY()

	// 生命值
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;

	// 攻击力
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Attack = 10.f;

	// 防御力 (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defence = 0.f;

	// 暴击率 (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalRate = 10.f;

	// 暴击伤害 (最小100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalDamage = 200.f;

	// 当前血量（-1 表示满血）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = -1.f;
};

/**
 * 战斗角色基类 - 拥有 GAS 系统的角色
 * 包含：AbilitySystemComponent、AttributeSet、ISHCombatInterface 实现
 *
 * ASC 所有权：
 * - 默认自己拥有 ASC（适用于敌人角色）
 * - 子类可覆盖 GetAbilitySystemComponent() 从其他来源获取（如 PlayerState）
 */
UCLASS(Abstract)
class SAILINGHEART_API ASHCombatCharacterBase : public ASHCharacterBase, public IAbilitySystemInterface, public ISHCombatInterface
{
	GENERATED_BODY()

public:
	ASHCombatCharacterBase(const FObjectInitializer& ObjectInitializer);

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ========== 属性访问器 ==========

	// 获取属性集（基类指针，子类可能返回更具体的类型）
	UFUNCTION(BlueprintCallable, Category = "GAS")
	USHAttributeSetBase* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetDefence() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetCriticalRate() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetCriticalDamage() const;

	// ISHCombatInterface 实现（IsDead, Die, GetAvatar）
	virtual bool IsDead_Implementation() const override;
	virtual void Die_Implementation() override;
	virtual AActor* GetAvatar_Implementation() override;

	// ========== 死亡处理 ==========

	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void HandleDeath();
	virtual void HandleDeath_Implementation();

protected:
	virtual void BeginPlay() override;

	// ========== GAS 组件 ==========

	// AbilitySystemComponent（自己拥有，敌人角色使用）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	// 属性集（基类指针）
	UPROPERTY()
	USHAttributeSetBase* AttributeSet;

	// 使用的属性集类（子类可在构造函数中设置不同的类）
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<USHAttributeSetBase> AttributeSetClass;

	// ========== 初始化 ==========

	// 初始化角色（属性 + 能力）
	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void InitializeCharacter(const FCharacterInitParams& Params);

	// 初始化属性
	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void InitializeAttributes(const FCharacterInitParams& Params);

	// 死亡回调
	UFUNCTION()
	void OnDeathCallback(AActor* DeadActor);

	// 死亡状态标记（防止重复调用 HandleDeath）
	bool bIsDying = false;

	// ========== 调试 ==========

	virtual void DrawDebugInfo() override;
};
