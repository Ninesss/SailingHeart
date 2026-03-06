// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SHAbilityDataBase.generated.h"

class UGameplayAbility;
class UTexture2D;
class UStaticMesh;
struct FSHAbilityParams;

/**
 * 主动能力数据基类
 * 定义能力的基本信息和 UI 展示
 * 子类添加具体的参数配置
 */
UCLASS(Abstract, BlueprintType)
class SAILINGHEART_API USHAbilityDataBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ========== 基本信息 ==========

	// 能力 ID（唯一标识）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
	FName AbilityID;

	// 显示名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
	FText DisplayName;

	// 描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
	FText Description;

	// 图标（UI 用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
	TSoftObjectPtr<UTexture2D> Icon;

	// ========== 3D 展示 ==========

	// 拾取物/商店展示用的 Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Display")
	TSoftObjectPtr<UStaticMesh> DisplayMesh;

	// Mesh 的材质（可选覆盖）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Display")
	TSoftObjectPtr<UMaterialInterface> DisplayMaterial;

	// ========== 能力配置 ==========

	// 能力类（GameplayAbility）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Class")
	TSubclassOf<UGameplayAbility> AbilityClass;

	// ========== 辅助函数 ==========

	// 获取最大等级（子类实现）
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual int32 GetMaxLevel() const { return 0; }

	// 检查等级是否有配置（子类实现）
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool HasLevelConfig(int32 Level) const { return false; }

	// 获取基类参数（用于Cost/Cooldown等通用属性）
	// 子类实现返回具体参数的基类指针
	virtual const FSHAbilityParams* GetBaseParams(int32 Level) const { return nullptr; }

	// 获取 PrimaryAssetId（用于资产管理）
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ActiveAbility", GetFName());
	}
};
