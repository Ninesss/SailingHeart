// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SHEnemyCharacterData.generated.h"

class ASHEnemyCharacterBase;
class UStateTree;
class USHAbilityDataBase;

/**
 * 敌人角色数据资源
 * 每种敌人类型一个 DataAsset 文件
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHEnemyCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 敌人类型 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FName EnemyTypeID;

	// 敌人显示名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FText DisplayName;

	// 敌人蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TSubclassOf<ASHEnemyCharacterBase> EnemyClass;

	// StateTree 行为树（用于 AI 行为控制）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UStateTree> StateTree;

	// ========== 初始属性 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Attack = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Defence = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float CriticalRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float CriticalDamage = 200.f;

	// ========== 初始能力 ==========

	// 敌人的初始能力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<USHAbilityDataBase*> InitialAbilities;
};
