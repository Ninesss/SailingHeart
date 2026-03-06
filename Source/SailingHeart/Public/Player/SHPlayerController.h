// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SHPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ASHGridBase;
class ASHPlayerBlock;
class USHPlayerCharacterData;
class ASHPlayerState;

/**
 * 玩家控制器
 */
UCLASS()
class SAILINGHEART_API ASHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASHPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 获取本地玩家 ID
	UFUNCTION(BlueprintCallable, Category = "Player")
	FString GetLocalPlayerID() const;

	// ========== 角色选择 ==========

	// 获取已选角色 ID
	UFUNCTION(BlueprintCallable, Category = "Character")
	FName GetSelectedCharacterID() const { return SelectedCharacterID; }

	// 选择角色（客户端调用，发送到服务器）
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SelectCharacter(FName CharacterID);

	// 检查是否已选择角色
	UFUNCTION(BlueprintCallable, Category = "Character")
	bool HasSelectedCharacter() const { return !SelectedCharacterID.IsNone(); }

	// 获取当前角色数据（从 GameState 查找）
	UFUNCTION(BlueprintCallable, Category = "Character")
	USHPlayerCharacterData* GetSelectedCharacterData() const;

	// ========== 能力系统 ==========

	// 获取 SH PlayerState
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	ASHPlayerState* GetSHPlayerState() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 已选择的角色 ID（服务器权威）
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
	FName SelectedCharacterID;

private:
	// ========== 输入动作 ==========

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PlaceBlockAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> CarryBlockAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DebugShowBlocksAction;

	// 能力触发（鼠标左键 / 手柄按键）
	UPROPERTY(EditAnywhere, Category="Input|Ability")
	TObjectPtr<UInputAction> TriggerAbilityAction;

	// 切换能力槽（鼠标右键 / 手柄按键）
	UPROPERTY(EditAnywhere, Category="Input|Ability")
	TObjectPtr<UInputAction> SwitchAbilitySlotAction;

	// 可生成的方块类型 ID 列表（对应 DataTable 中的 Row Name）
	UPROPERTY(EditAnywhere, Category="ShipBlock")
	TArray<FName> AvailableBlockTypes;

	// ========== 输入处理函数 ==========

	void Move(const FInputActionValue& InputActionValue);
	void StartJump();
	void StopJump();
	void PlaceBlock();
	void CarryBlock();
	void DebugShowAllBlocks();

	// 能力相关
	void TriggerAbility();
	void SwitchAbilitySlot();

	// ========== 服务器 RPC ==========

	// 服务器RPC - 在服务器上生成船方块
	UFUNCTION(Server, Reliable)
	void ServerPlaceBlock();

	// 服务器RPC - 尝试拾取或放下船方块
	UFUNCTION(Server, Reliable)
	void ServerCarryBlock();

	// 服务器RPC - 触发当前能力
	UFUNCTION(Server, Reliable)
	void ServerTriggerAbility();

	// 服务器RPC - 切换能力槽
	UFUNCTION(Server, Reliable)
	void ServerSwitchAbilitySlot();

	// 服务器RPC - 设置选择的角色
	UFUNCTION(Server, Reliable)
	void ServerSelectCharacter(FName CharacterID);

	// ========== 客户端 RPC ==========

	// 客户端RPC - 通知客户端单元格已有船方块
	UFUNCTION(Client, Reliable)
	void ClientNotifyBlockExists();

	// 客户端RPC - 通知能力触发失败
	UFUNCTION(Client, Reliable)
	void ClientNotifyAbilityFailed(const FString& Reason);
};
