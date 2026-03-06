// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Block/SHBlockBase.h"
#include "AbilitySystemInterface.h"
#include "Interface/SHCombatInterface.h"
#include "SHCombatBlockBase.generated.h"

class USHBlockAttributeSet;
class UAbilitySystemComponent;
class USHAbilityDataBase;
class UGameplayEffect;

/**
 * 方块等级配置 - 通用结构体，用于 PlayerBlock 和 EnemyBlock
 */
USTRUCT(BlueprintType)
struct FBlockLevelConfig
{
	GENERATED_BODY()

	// 最大血量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxHealth = 100.f;
	
	// 最大能量（0 = 无能量系统）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxEnergy = 100.f;

	// 能量回复速度（每秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float EnergyRegen = 1.f;

	// 攻击力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Attack = 10.f;

	// 防御力（百分比减伤）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "0", ClampMax = "100"))
	float Defence = 0.f;

	// 暴击率（百分比）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "0", ClampMax = "100"))
	float CriticalRate = 10.f;

	// 暴击伤害（百分比，150 = 1.5倍伤害）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "100"))
	float CriticalDamage = 200.f;

	// 行动速度（百分比，100 = 正常速度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "1"))
	float ActionSpeed = 100.f;

	// 该等级激活的技能（使用 DataAsset 配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<USHAbilityDataBase*> GrantedAbilities;
};

/**
 * 方块初始化参数 - 统一所有战斗方块的初始化流程
 */
USTRUCT(BlueprintType)
struct FBlockInitParams
{
	GENERATED_BODY()

	// 等级
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

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

	// 行动速度 (最小1%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActionSpeed = 100.f;

	// 当前血量（-1 表示使用 MaxHealth，即满血）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = -1.f;

	// 最大能量（0 = 无能量系统）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxEnergy = 0.f;

	// 能量回复速度（每秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnergyRegen = 0.f;

	// 初始授予的能力（使用 DataAsset 配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USHAbilityDataBase*> AbilityDataAssets;
};

/**
 * 战斗方块基类 - 拥有 GAS 系统的方块
 * 包含：AbilitySystemComponent、AttributeSet、属性访问器、能力管理
 */
UCLASS(Abstract)
class SAILINGHEART_API ASHCombatBlockBase : public ASHBlockBase, public IAbilitySystemInterface, public ISHCombatInterface
{
	GENERATED_BODY()

public:
	ASHCombatBlockBase();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	// ========== 属性访问器 ==========

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetBlockLevel() const;

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

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetActionSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetEnergyRegen() const;

	// ISHCombatInterface 实现（IsDead, Die, GetAvatar）
	virtual bool IsDead_Implementation() const override;
	virtual void Die_Implementation() override;
	virtual AActor* GetAvatar_Implementation() override;

	// ========== 死亡处理 ==========

	// 处理死亡（子类可覆盖添加特效等）
	UFUNCTION(BlueprintNativeEvent, Category = "Block")
	void HandleDeath();
	virtual void HandleDeath_Implementation();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ========== GAS 组件 ==========

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	USHBlockAttributeSet* AttributeSet;

	// 能量回复 GE（在蓝图子类中设置，Infinite + Period）
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> EnergyRegenEffectClass;

	// ========== 统一初始化入口 ==========

	/**
	 * 统一的方块初始化方法
	 * 子类应该使用此方法进行初始化，确保属性、能力、碰撞都被正确设置
	 * @param Params 初始化参数
	 */
	UFUNCTION(BlueprintCallable, Category = "Block")
	void InitializeBlock(const FBlockInitParams& Params);

	// ========== 底层初始化方法 ==========

	// 初始化属性（使用 FBlockInitParams 结构体）
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void InitializeAttributes(const FBlockInitParams& Params);

	// 授予技能（使用 DataAsset）
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GrantAbilitiesFromData(const TArray<USHAbilityDataBase*>& AbilityDataAssets, int32 Level = 1);

	// 清除所有技能
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void ClearAbilities();

	// 死亡回调（绑定到 AttributeSet 的 OnDeath）
	UFUNCTION()
	void OnDeathCallback(AActor* DeadActor);

	// 死亡状态标记（防止重复调用 HandleDeath）
	bool bIsDying = false;

	// ========== 调试 ==========

	virtual void DrawDebugInfo() override;

	// 子类可覆盖：添加额外的调试信息（会插入在属性和技能之间）
	virtual FString GetExtraDebugText() const { return FString(); }

	// 子类可覆盖：设置调试文字颜色
	virtual FColor GetDebugColor() const { return FColor::White; }
};
