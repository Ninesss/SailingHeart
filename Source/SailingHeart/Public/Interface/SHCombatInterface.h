// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"
#include "SHCombatInterface.generated.h"

/**
 * 战斗接口 - 所有具有战斗能力的 Actor 实现此接口
 * 提供死亡处理、Avatar 访问和战斗 Socket 位置查询
 *
 * 注意：属性（Health, Attack, Defence 等）直接通过 AttributeSet 访问，不走接口
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USHCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class ISHCombatInterface
{
	GENERATED_BODY()

public:
	// ========== 死亡状态 ==========

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsDead() const;

	// ========== 死亡处理 ==========

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void Die();

	// ========== Avatar ==========

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	AActor* GetAvatar();

	// ========== 战斗 Socket ==========

	/**
	 * 通过 Socket 标签获取 SKM 上对应 Socket 的世界变换
	 * 供 AnimNotify 读取发射点位置，标签定义在 FSHGameplayTags::CombatSocket_*
	 * @param SocketTag  CombatSocket.Weapon / CombatSocket.LeftHand 等
	 * @return Socket 的世界变换，找不到时返回 Actor 变换
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	FTransform GetCombatSocketTransform(const FGameplayTag& SocketTag) const;

	// ========== 动画 Montage ==========

	/**
	 * 根据技能 TriggerTag 返回对应的 AnimMontage
	 * Actor（Block/Character 蓝图子类）在 AbilityMontageMap 中配置具体的 Montage
	 * 技能本身不保存 Montage，保证技能与角色解耦
	 * @param TriggerTag  技能触发 Tag（Ability.Trigger.*），与 AbilityDataBase.TriggerTag 一致
	 * @return 对应的 AnimMontage，未配置时返回 nullptr
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	UAnimMontage* GetAbilityMontage(const FGameplayTag& TriggerTag) const;
};
