// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"
#include "Data/SHAIConfig.h"
#include "SHAIEvaluator.generated.h"

/**
 * SHAIEvaluator 的 Instance Data
 * 每个 StateTree 实例独立持有，用于存储运行时状态
 */
USTRUCT()
struct FSHAIEvaluatorInstanceData
{
	GENERATED_BODY()

	// ===== 输出（供 Task 和 Condition 绑定读取）=====

	// 当前锁定的目标（检测范围内最近的敌对单位）
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// 到当前目标的距离
	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget = 0.f;

	// 目标是否在检测范围内
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bInDetectionRange = false;

	// 目标是否在攻击范围内
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bInAttackRange = false;
};

/**
 * AI 目标检测 Evaluator
 * 每隔一定时间做球形检测，找到最近敌对单位并输出到上下文
 * 友军（Ally）寻找 Enemy；Enemy 寻找 Player 和 Ally
 *
 * 用法：在 StateTree 资产的 Root 层级加入此 Evaluator，所有状态的 Task/Condition
 *       通过数据绑定读取 CurrentTarget、bInDetectionRange、bInAttackRange
 */
USTRUCT(meta = (DisplayName = "SH AI Target Evaluator"))
struct SAILINGHEART_API FSHAIEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSHAIEvaluatorInstanceData;

	// Evaluator 检测间隔（秒），降低 Tick 频率节省性能
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.05"))
	float TickInterval = 0.2f;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FSHAIEvaluatorInstanceData::StaticStruct();
	}

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	// 执行球形检测，更新 InstanceData
	void UpdateTarget(FStateTreeExecutionContext& Context) const;

	// 累积 Tick 计时器（存在 InstanceData 外部，通过 mutable 模拟）
	// 注意：实际计时存储在独立的 InstanceData 扩展或通过 Context 获取
	// 此处使用简化方案：每次 Tick 直接判断时间
};
