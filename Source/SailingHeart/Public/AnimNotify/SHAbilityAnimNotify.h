// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "SHAbilityAnimNotify.generated.h"

/**
 * 技能触发 AnimNotify
 * 动画中放置此 Notify，触发时向 Actor 发送 GameplayEvent
 * Ability 通过 UAbilityTask_WaitGameplayEvent 监听该 Event 执行攻击逻辑
 *
 * 同时把 FunctionalSKM 上对应 Socket 的世界变换打包进 TargetData，
 * 供 Ability 读取发射点位置（通过 ISHCombatInterface::GetCombatSocketTransform）
 */
UCLASS()
class SAILINGHEART_API USHAbilityAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	/**
	 * Socket 事件标签，使用 CombatSocket.* 命名空间（如 CombatSocket.Weapon）
	 * 两用：
	 * 1. 向 ISHCombatInterface::GetCombatSocketTransform 查询发射点位置
	 * 2. 作为 GameplayEvent Tag 发送给 ASC，Ability 通过 WaitGameplayEvent 监听此 Tag
	 *
	 * 注意：与激活技能的 TriggerTag（Ability.Trigger.*）不同
	 * - TriggerTag（Ability.Trigger.*）：StateTree 触发技能时使用，技能通过 AbilityTriggers 响应
	 * - EventTag（CombatSocket.*）：Montage 关键帧发送，Ability 在播放 Montage 时监听，用于确定发射时机和位置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityNotify")
	FGameplayTag EventTag;

	// fallback Socket 名称（Actor 未实现 ISHCombatInterface 时使用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityNotify")
	FName FallbackSocketName = FName("WeaponSocket");

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
