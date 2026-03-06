// Sailing Heart


#include "Player/SHPlayerController.h"
#include "Player/SHPlayerState.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Character/SHPlayerCharacterBase.h"
#include "Grid/SHGridBase.h"
#include "Block/SHBlockBase.h"
#include "Block/SHPlayerBlock.h"
#include "Block/SHNeutralBlock.h"
#include "Game/SHGameStateBase.h"
#include "Data/SHPlayerBlockData.h"
#include "Data/SHPlayerCharacterData.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "SHDebugLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASHPlayerController::ASHPlayerController()
{
	bReplicates = true;
}

void ASHPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHPlayerController, SelectedCharacterID);
}

void ASHPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(PlayerInputMappingContext);

	// 绑定IMC
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->AddMappingContext(PlayerInputMappingContext, 0);
		}
	}

	// 显示鼠标
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	// 设置input mode
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

FString ASHPlayerController::GetLocalPlayerID() const
{
	return FPlatformMisc::GetDeviceId();
}

ASHPlayerState* ASHPlayerController::GetSHPlayerState() const
{
	return GetPlayerState<ASHPlayerState>();
}

void ASHPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASHPlayerController::Move);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASHPlayerController::StartJump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASHPlayerController::StopJump);
	EnhancedInputComponent->BindAction(PlaceBlockAction, ETriggerEvent::Started, this, &ASHPlayerController::PlaceBlock);
	EnhancedInputComponent->BindAction(CarryBlockAction, ETriggerEvent::Started, this, &ASHPlayerController::CarryBlock);

	if (DebugShowBlocksAction)
	{
		EnhancedInputComponent->BindAction(DebugShowBlocksAction, ETriggerEvent::Started, this, &ASHPlayerController::DebugShowAllBlocks);
	}

	if (TriggerAbilityAction)
	{
		EnhancedInputComponent->BindAction(TriggerAbilityAction, ETriggerEvent::Started, this, &ASHPlayerController::TriggerAbility);
	}

	if (SwitchAbilitySlotAction)
	{
		EnhancedInputComponent->BindAction(SwitchAbilitySlotAction, ETriggerEvent::Started, this, &ASHPlayerController::SwitchAbilitySlot);
	}
}

void ASHPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void ASHPlayerController::StartJump()
{
	if (ACharacter* PlayerCharacter = GetPawn<ACharacter>())
	{
		PlayerCharacter->Jump();
	}
}

void ASHPlayerController::StopJump()
{
	if (ACharacter* PlayerCharacter = GetPawn<ACharacter>())
	{
		PlayerCharacter->StopJumping();
	}
}

void ASHPlayerController::PlaceBlock()
{
	if (AvailableBlockTypes.Num() == 0)
	{
		return;
	}

	ServerPlaceBlock();
}

void ASHPlayerController::ServerPlaceBlock_Implementation()
{
	if (AvailableBlockTypes.Num() == 0)
	{
		return;
	}

	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return;
	}

	FVector Location;
	int32 Row, Column;
	ASHGridBase* TargetGrid = ASHGridBase::GetPlayerFacingCellInfo(this, MyPawn, Location, Row, Column);
	if (!TargetGrid)
	{
		return;
	}

	if (!TargetGrid->IsCellPlaceable(Row, Column))
	{
		return;
	}

	if (TargetGrid->HasShipBlockAt(Row, Column))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ASHGameStateBase* GS = GetWorld()->GetGameState<ASHGameStateBase>();
	if (!GS)
	{
		return;
	}

	int32 RandomIndex = FMath::RandRange(0, AvailableBlockTypes.Num() - 1);
	FName SelectedBlockTypeID = AvailableBlockTypes[RandomIndex];

	USHPlayerBlockData* BlockData = GS->GetPlayerBlockData(SelectedBlockTypeID);
	if (!BlockData || !BlockData->BlockClass || !BlockData->IsLevelValid(1))
	{
		return;
	}

	FBlockLevelConfig LevelConfig = BlockData->GetLevelConfig(1);

	// 使用统一的 Deferred 生成方法
	ASHPlayerBlock::SpawnDeferred(
		World,
		BlockData->BlockClass,
		TargetGrid,
		Row, Column,
		SelectedBlockTypeID,
		1,
		LevelConfig,
		-1.f,
		BlockData->StateTree
	);
}

void ASHPlayerController::CarryBlock()
{
	ServerCarryBlock();
}

void ASHPlayerController::ServerCarryBlock_Implementation()
{
	ASHPlayerCharacterBase* PlayerCharacter = GetPawn<ASHPlayerCharacterBase>();
	if (!PlayerCharacter)
	{
		return;
	}

	FVector Location;
	int32 Row, Column;
	ASHGridBase* TargetGrid = ASHGridBase::GetPlayerFacingCellInfo(this, PlayerCharacter, Location, Row, Column);
	if (!TargetGrid)
	{
		return;
	}

	if (PlayerCharacter->IsCarryingBlock())
	{
		if (!TargetGrid->IsCellPlaceable(Row, Column))
		{
			return;
		}

		FBlockCarryState CarriedState = PlayerCharacter->GetCarriedBlockState();

		ASHGameStateBase* GS = GetWorld()->GetGameState<ASHGameStateBase>();
		if (!GS)
		{
			return;
		}

		USHPlayerBlockData* BlockData = GS->GetPlayerBlockData(CarriedState.BlockTypeID);
		if (!BlockData || !BlockData->BlockClass)
		{
			return;
		}

		if (TargetGrid->HasShipBlockAt(Row, Column))
		{
			ASHPlayerBlock* ExistingBlock = TargetGrid->GetShipBlockAt(Row, Column);

			int32 ExistingLevel = FMath::RoundToInt(ExistingBlock->GetBlockLevel());
			if (ExistingBlock->GetBlockTypeID() == CarriedState.BlockTypeID &&
				ExistingLevel == CarriedState.Level)
			{
				int32 NewLevel = ExistingLevel + 1;

				if (BlockData->IsLevelValid(NewLevel))
				{
					FBlockLevelConfig NewLevelConfig = BlockData->GetLevelConfig(NewLevel);

					if (ExistingBlock->MergeWith(CarriedState, NewLevelConfig))
					{
						PlayerCharacter->StopCarrying();
						return;
					}
				}
			}

			ClientNotifyBlockExists();
			return;
		}

		FBlockLevelConfig LevelConfig = BlockData->GetLevelConfig(CarriedState.Level);

		// 使用统一的 Deferred 生成方法
		ASHPlayerBlock* NewBlock = ASHPlayerBlock::SpawnDeferred(
			GetWorld(),
			BlockData->BlockClass,
			TargetGrid,
			Row, Column,
			CarriedState.BlockTypeID,
			CarriedState.Level,
			LevelConfig,
			CarriedState.CurrentHealth,
			BlockData->StateTree
		);

		if (NewBlock)
		{
			PlayerCharacter->StopCarrying();
		}
	}
	else
	{
		ASHPlayerBlock* BlockInCell = TargetGrid->GetShipBlockAt(Row, Column);
		if (BlockInCell)
		{
			FBlockCarryState BlockState = BlockInCell->CreateCarryState();
			UStaticMesh* CarryMesh = BlockInCell->GetMesh();

			TargetGrid->ClearShipBlockAt(Row, Column);
			BlockInCell->Destroy();
			PlayerCharacter->StartCarrying(BlockState, CarryMesh);
		}
	}
}

void ASHPlayerController::ClientNotifyBlockExists_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("已有船方块"));
}

void ASHPlayerController::DebugShowAllBlocks()
{
	USHDebugLibrary::ToggleGlobalDebugDisplay();

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
		FString::Printf(TEXT("调试显示 %s"), USHDebugLibrary::GetGlobalDebugDisplay() ? TEXT("开启") : TEXT("关闭")));
}

// ========== 能力相关输入 ==========

void ASHPlayerController::TriggerAbility()
{
	ServerTriggerAbility();
}

void ASHPlayerController::ServerTriggerAbility_Implementation()
{
	ASHPlayerState* PS = GetSHPlayerState();
	if (!PS)
	{
		return;
	}

	const FSHAbilitySlot& CurrentSlot = PS->GetCurrentAbilitySlot();
	if (CurrentSlot.IsEmpty())
	{
		ClientNotifyAbilityFailed(TEXT("No ability equipped"));
		return;
	}

	if (!PS->CanTriggerCurrentAbility())
	{
		ClientNotifyAbilityFailed(TEXT("Ability on cooldown or already active"));
		return;
	}

	if (!PS->TriggerCurrentAbility())
	{
		ClientNotifyAbilityFailed(TEXT("Failed to activate ability (cost or blocked)"));
	}
}

void ASHPlayerController::SwitchAbilitySlot()
{
	ServerSwitchAbilitySlot();
}

void ASHPlayerController::ServerSwitchAbilitySlot_Implementation()
{
	ASHPlayerState* PS = GetSHPlayerState();
	if (!PS)
	{
		return;
	}

	PS->SwitchToNextSlot();
}

void ASHPlayerController::ClientNotifyAbilityFailed_Implementation(const FString& Reason)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Ability Failed: %s"), *Reason));
}

// ========== 角色选择 ==========

void ASHPlayerController::SelectCharacter(FName CharacterID)
{
	if (CharacterID.IsNone())
	{
		return;
	}

	ASHGameStateBase* GS = GetWorld()->GetGameState<ASHGameStateBase>();
	if (GS && GS->GetPlayerCharacterData(CharacterID))
	{
		ServerSelectCharacter(CharacterID);
	}
}

void ASHPlayerController::ServerSelectCharacter_Implementation(FName CharacterID)
{
	if (CharacterID.IsNone())
	{
		return;
	}

	ASHGameStateBase* GS = GetWorld()->GetGameState<ASHGameStateBase>();
	if (!GS || !GS->GetPlayerCharacterData(CharacterID))
	{
		return;
	}

	SelectedCharacterID = CharacterID;
}

USHPlayerCharacterData* ASHPlayerController::GetSelectedCharacterData() const
{
	if (SelectedCharacterID.IsNone())
	{
		return nullptr;
	}

	ASHGameStateBase* GS = GetWorld()->GetGameState<ASHGameStateBase>();
	if (GS)
	{
		return GS->GetPlayerCharacterData(SelectedCharacterID);
	}
	return nullptr;
}
