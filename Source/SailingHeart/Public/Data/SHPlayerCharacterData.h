// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SHPlayerCharacterData.generated.h"

class ASHPlayerCharacterBase;
class UGameplayAbility;
class USHAbilityDataBase;
class USHPassiveAbilityData;

/**
 * 玩家角色数据资源
 * 每种角色类型一个 DataAsset 文件
 */
UCLASS(BlueprintType)
class SAILINGHEART_API USHPlayerCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 角色 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CharacterID;

	// 角色显示名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FText DisplayName;

	// 角色蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TSubclassOf<ASHPlayerCharacterBase> CharacterClass;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float EnergyRegen = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MovementSpeed = 100.f;

	// ========== 初始主动能力 ==========

	// 角色的初始主动能力（下标对应槽位，最多 2 个）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Active", meta = (TitleProperty = "AbilityID", MaxArraySize = 2))
	TArray<USHAbilityDataBase*> InitialActiveAbilities;

	// ========== 初始被动能力 ==========

	// 角色的初始被动能力（永久生效）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Passive")
	USHPassiveAbilityData* InitialPassiveAbility = nullptr;
};
