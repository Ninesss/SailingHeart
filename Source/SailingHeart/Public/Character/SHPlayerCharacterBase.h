// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Character/SHCombatCharacterBase.h"
#include "Save/SHBlockTypes.h"
#include "SHPlayerCharacterBase.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USHCharacterMovementComponent;
class USHPlayerAttributeSet;

/**
 * 玩家角色基类
 * 继承自 SHCombatCharacterBase，使用 PlayerState 的 ASC
 */
UCLASS()
class SAILINGHEART_API ASHPlayerCharacterBase : public ASHCombatCharacterBase
{
	GENERATED_BODY()

public:
	ASHPlayerCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// 覆盖：从 PlayerState 获取 ASC
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 获取玩家属性集（比基类更具体）
	UFUNCTION(BlueprintCallable, Category = "GAS")
	USHPlayerAttributeSet* GetPlayerAttributeSet() const;

	// ========== 玩家特有属性 ==========

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxEnergy() const;

	// 获取自定义移动组件
	UFUNCTION(BlueprintCallable, Category = "Movement")
	USHCharacterMovementComponent* GetCustomMovement() const { return CustomMovementComponent; }

	// 重写跳跃检查以支持 Coyote Time
	virtual bool CanJumpInternal_Implementation() const override;

	// ========== 搬运相关 ==========

	UFUNCTION(BlueprintCallable, Category = "Carry")
	bool IsCarryingBlock() const { return bIsCarrying; }

	UFUNCTION(BlueprintCallable, Category = "Carry")
	void StartCarrying(const FBlockCarryState& InBlockState, UStaticMesh* InCarryMesh);

	UFUNCTION(BlueprintCallable, Category = "Carry")
	void StopCarrying();

	UFUNCTION(BlueprintCallable, Category = "Carry")
	FBlockCarryState GetCarriedBlockState() const { return CarriedBlockState; }

protected:
	virtual void BeginPlay() override;
	virtual void DrawDebugInfo() override;

	// 初始化 ASC（从 PlayerState 获取）
	void InitializeAbilitySystem();

	// 缓存的 PlayerState ASC 引用
	UPROPERTY()
	UAbilitySystemComponent* PlayerStateASC;

	// 缓存的玩家属性集引用
	UPROPERTY()
	USHPlayerAttributeSet* PlayerAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	USHCharacterMovementComponent* CustomMovementComponent;

	// 相机
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// 搬运视觉组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carry")
	TObjectPtr<UStaticMeshComponent> CarryMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
	float CarryHeightOffset = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
	float CarryMeshScale = 0.9f;

private:
	UPROPERTY(ReplicatedUsing = OnRep_CarryState)
	bool bIsCarrying = false;

	UPROPERTY(ReplicatedUsing = OnRep_CarryState)
	UStaticMesh* CurrentCarryMesh = nullptr;

	UPROPERTY(Replicated)
	FBlockCarryState CarriedBlockState;

	UFUNCTION()
	void OnRep_CarryState();

	void UpdateCarryVisual();
};
