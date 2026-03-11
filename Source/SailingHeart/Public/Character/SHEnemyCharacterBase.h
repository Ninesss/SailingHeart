// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Character/SHCombatCharacterBase.h"
#include "Components/StateTreeComponent.h"
#include "AI/SHAIEntityInterface.h"
#include "SHEnemyCharacterBase.generated.h"

class USHEnemyCharacterData;

/**
 * 敌人角色基类
 * 继承自 SHCombatCharacterBase，自己拥有 ASC，通过 StateTree 执行 AI 行为
 */
UCLASS()
class SAILINGHEART_API ASHEnemyCharacterBase : public ASHCombatCharacterBase, public ISHAIEntityInterface
{
	GENERATED_BODY()

public:
	ASHEnemyCharacterBase(const FObjectInitializer& ObjectInitializer);

	// ========== 属性 ==========

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	FName GetEnemyTypeID() const { return CharacterTypeID; }

	// ========== 组件 ==========

	UFUNCTION(BlueprintCallable, Category = "Components")
	UStateTreeComponent* GetStateTreeComponent() const { return StateTreeComponent; }

	// ========== 初始化 ==========

	/**
	 * 使用 DataAsset 初始化敌人属性
	 * @param EnemyData 敌人数据资产
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeFromData(USHEnemyCharacterData* EnemyData);

	// ISHAIEntityInterface
	virtual const FSHAIConfig& GetAIConfig() const override;

protected:
	virtual void BeginPlay() override;
	virtual void DrawDebugInfo() override;

	// StateTree 组件（用于 AI 行为控制）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStateTreeComponent* StateTreeComponent;

	// 缓存的敌人数据资产引用
	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<USHEnemyCharacterData> EnemyCharacterData;

	// 默认 AI 配置（EnemyCharacterData 未设置时使用）
	FSHAIConfig DefaultAIConfig;
};
