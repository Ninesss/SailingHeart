// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SHCharacterBase.generated.h"

UCLASS()
class SAILINGHEART_API ASHCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASHCharacterBase(const FObjectInitializer& ObjectInitializer);

	// ========== 基础访问器 ==========

	UFUNCTION(BlueprintCallable, Category = "Character")
	FName GetCharacterTypeID() const { return CharacterTypeID; }

	// ========== 调试 ==========

	// 设置全局调试显示状态
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void SetGlobalShowDebugAttributes(bool bShow);

	// 获取全局调试显示状态
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static bool GetGlobalShowDebugAttributes() { return bGlobalShowDebugInfo; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 角色类型 ID
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
	FName CharacterTypeID;

	// 绘制调试信息（子类覆盖实现）
	virtual void DrawDebugInfo();

private:
	static bool bGlobalShowDebugInfo;
};
