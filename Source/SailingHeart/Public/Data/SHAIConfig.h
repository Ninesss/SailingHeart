// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "SHAIConfig.generated.h"

/**
 * AI 行为配置 - 所有拥有 StateTree 的战斗单位共用
 * 嵌入到对应的 DataAsset 中（EnemyBlockData、PlayerBlockData、EnemyCharacterData 等）
 */
USTRUCT(BlueprintType)
struct FSHAIConfig
{
	GENERATED_BODY()

	// 检测范围：进入此范围后开始凝视/追击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0"))
	float DetectionRange = 1500.f;

	// 攻击范围：进入此范围后触发技能
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0"))
	float AttackRange = 800.f;

	// 是否可以移动追击目标（true = 角色类，false = 方块类）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bCanMove = false;
};
