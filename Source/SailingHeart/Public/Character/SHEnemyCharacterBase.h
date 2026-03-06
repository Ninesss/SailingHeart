// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Character/SHCombatCharacterBase.h"
#include "Components/StateTreeComponent.h"
#include "SHEnemyCharacterBase.generated.h"

class USHEnemyCharacterData;

/**
 * 敌人角色基类
 * 继承自 SHCombatCharacterBase，自己拥有 ASC
 */
UCLASS()
class SAILINGHEART_API ASHEnemyCharacterBase : public ASHCombatCharacterBase
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
	 * 使用 DataAsset 初始化敌人（属性 + StateTree）
	 * @param EnemyData 敌人数据资产
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeFromData(USHEnemyCharacterData* EnemyData);

protected:
	virtual void BeginPlay() override;
	virtual void DrawDebugInfo() override;

	// StateTree 组件（用于 AI 行为控制）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStateTreeComponent* StateTreeComponent;

	// 缓存的敌人数据资产引用
	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<USHEnemyCharacterData> EnemyCharacterData;
};
