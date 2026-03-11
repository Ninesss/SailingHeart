// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/SHAIConfig.h"
#include "SHAIEntityInterface.generated.h"

/**
 * AI 实体接口 - 所有拥有 StateTree AI 的战斗单位实现此接口
 * 提供 AI 配置（检测/攻击范围）访问入口，供 StateTree Evaluator/Task 使用
 *
 * 实现者：ASHPlayerBlock、ASHEnemyBlock、ASHEnemyCharacterBase、（未来）ASHAllyCharacterBase
 */
UINTERFACE(MinimalAPI, BlueprintType)
class USHAIEntityInterface : public UInterface
{
	GENERATED_BODY()
};

class SAILINGHEART_API ISHAIEntityInterface
{
	GENERATED_BODY()

public:
	// 返回该单位的 AI 配置（范围、移动能力等）
	virtual const FSHAIConfig& GetAIConfig() const = 0;
};
