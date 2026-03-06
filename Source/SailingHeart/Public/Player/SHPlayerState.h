// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "ActiveGameplayEffectHandle.h"
#include "SHPlayerState.generated.h"

class UAbilitySystemComponent;
class USHPlayerAttributeSet;
class USHPlayerCharacterData;
class USHAbilityDataBase;
class USHPassiveAbilityData;
class UGameplayEffect;

/**
 * 能力槽结构 - 存储装备的主动能力信息
 */
USTRUCT(BlueprintType)
struct FSHAbilitySlot
{
	GENERATED_BODY()

	// 能力数据资产（复制到客户端用于 UI 显示）
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	USHAbilityDataBase* AbilityData = nullptr;

	// 能力规格句柄（仅服务器使用，不复制）
	// 用于在服务器上激活/移除能力
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	// 是否为空槽
	bool IsEmpty() const { return AbilityData == nullptr; }

	// 清空槽
	void Clear()
	{
		AbilityData = nullptr;
		AbilitySpecHandle = FGameplayAbilitySpecHandle();
	}
};

/**
 * 玩家状态 - 持有 ASC 以支持多人游戏
 * ASC 放在 PlayerState 是因为：
 * 1. PlayerState 跨 Pawn 重生持久化
 * 2. 每个玩家有独立的 PlayerState
 * 3. 自动网络复制
 */
UCLASS()
class SAILINGHEART_API ASHPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASHPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	// 获取属性集
	UFUNCTION(BlueprintCallable, Category = "GAS")
	USHPlayerAttributeSet* GetPlayerAttributeSet() const { return AttributeSet; }

	// ========== 属性访问器 ==========

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetDefence() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxEnergy() const;

	// ========== 角色选择 ==========

	/** 获取当前选择的角色 ID */
	UFUNCTION(BlueprintCallable, Category = "Character")
	FName GetSelectedCharacterID() const { return SelectedCharacterID; }

	/** 客户端请求选择角色（Server RPC） */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Character")
	void ServerSelectCharacter(FName CharacterID);

	// ========== 初始化 ==========

	/**
	 * 设置 AvatarActor（当角色被 Possess 时调用）
	 * 必须在装备能力之前调用，否则能力会使用错误的 ActorInfo
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void SetAvatarActor(AActor* NewAvatar);

	/**
	 * 用角色数据初始化属性和能力
	 * @param CharacterData 角色 DataAsset
	 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void InitializeFromCharacterData(USHPlayerCharacterData* CharacterData);

	// ========== 能力管理工具函数 ==========

	// 清除所有能力
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void ClearAbilities();

	// ========== 主动能力槽系统 ==========

	// 能力槽数量
	static constexpr int32 MAX_ABILITY_SLOTS = 2;

	// 获取当前选中槽索引
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	// 获取指定槽的能力数据
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	const FSHAbilitySlot& GetAbilitySlot(int32 SlotIndex) const;

	// 获取当前选中槽的能力数据
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	const FSHAbilitySlot& GetCurrentAbilitySlot() const;

	// 装备能力到指定槽（服务器调用）
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	bool EquipAbility(USHAbilityDataBase* AbilityData, int32 SlotIndex);

	// 卸下指定槽的能力（服务器调用）
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	void UnequipAbility(int32 SlotIndex);

	// 切换到下一个槽
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	void SwitchToNextSlot();

	// 切换到指定槽
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	void SwitchToSlot(int32 SlotIndex);

	// 触发当前槽的能力
	// 返回是否成功触发
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	bool TriggerCurrentAbility();

	// 检查当前能力是否可用（冷却完成、未激活中）
	// Cost 检查由能力的 GameplayEffect 处理
	UFUNCTION(BlueprintCallable, Category = "AbilitySlots")
	bool CanTriggerCurrentAbility() const;

	// ========== 被动能力 ==========

	// 应用被动能力（永久 GE）
	UFUNCTION(BlueprintCallable, Category = "PassiveAbility")
	void ApplyPassiveAbility(USHPassiveAbilityData* PassiveData);

	// 移除被动能力
	UFUNCTION(BlueprintCallable, Category = "PassiveAbility")
	void RemovePassiveAbility();

	// 获取当前被动能力数据
	UFUNCTION(BlueprintCallable, Category = "PassiveAbility")
	USHPassiveAbilityData* GetPassiveAbilityData() const { return CurrentPassiveData; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	USHPlayerAttributeSet* AttributeSet;

	// 能量回复 GE（在蓝图中设置，Infinite + Period）
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> EnergyRegenEffectClass;

	// ========== 角色选择 ==========

	/** 玩家选择的角色 ID（服务器权威，复制到客户端） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
	FName SelectedCharacterID;

	// ========== 能力槽数据（服务器权威，复制到客户端） ==========

	// 能力槽数组
	UPROPERTY(ReplicatedUsing = OnRep_AbilitySlots, BlueprintReadOnly, Category = "AbilitySlots")
	TArray<FSHAbilitySlot> AbilitySlots;

	// 当前选中的槽索引
	UPROPERTY(ReplicatedUsing = OnRep_CurrentSlotIndex, BlueprintReadOnly, Category = "AbilitySlots")
	int32 CurrentSlotIndex = 0;

	// 当前被动能力数据
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PassiveAbility")
	USHPassiveAbilityData* CurrentPassiveData = nullptr;

	// 被动效果句柄
	FActiveGameplayEffectHandle PassiveEffectHandle;

	// ========== 复制回调 ==========

	UFUNCTION()
	void OnRep_AbilitySlots();

	UFUNCTION()
	void OnRep_CurrentSlotIndex();

private:
	// 空槽（用于返回引用）
	static FSHAbilitySlot EmptySlot;
};
