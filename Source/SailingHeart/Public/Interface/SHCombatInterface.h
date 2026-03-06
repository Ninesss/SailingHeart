// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SHCombatInterface.generated.h"

/**
 * 战斗接口 - 所有具有战斗能力的 Actor 实现此接口
 * 提供死亡处理和 Avatar 访问
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
};
