// Sailing Heart

#include "Character/SHPlayerCharacterBase.h"
#include "Character/SHCharacterMovementComponent.h"
#include "Player/SHPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/SHPlayerAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

ASHPlayerCharacterBase::ASHPlayerCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USHCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 缓存自定义移动组件引用
	CustomMovementComponent = Cast<USHCharacterMovementComponent>(GetCharacterMovement());

	// 玩家角色不自己拥有 ASC（从 PlayerState 获取）
	// 销毁基类创建的 ASC
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->DestroyComponent();
		AbilitySystemComponent = nullptr;
	}

	// Pawn 不跟着 Controller 旋转（由 MovementComponent 控制旋转）
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 相机臂设置
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1500.f;
	CameraBoom->SocketOffset = FVector(-300, 0, 1700);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// 相机设置
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 50.f;
	FollowCamera->SetRelativeRotation(FRotator(-40, 0, 0));

	// 搬运视觉组件
	CarryMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarryMeshComponent"));
	CarryMeshComponent->SetupAttachment(RootComponent);
	CarryMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, CarryHeightOffset));
	CarryMeshComponent->SetRelativeScale3D(FVector(CarryMeshScale));
	CarryMeshComponent->SetVisibility(false);
	CarryMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASHPlayerCharacterBase::BeginPlay()
{
	// 跳过基类的 ASC 初始化（我们从 PlayerState 获取）
	ASHCharacterBase::BeginPlay();
}

void ASHPlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHPlayerCharacterBase, bIsCarrying);
	DOREPLIFETIME(ASHPlayerCharacterBase, CurrentCarryMesh);
	DOREPLIFETIME(ASHPlayerCharacterBase, CarriedBlockState);
}

void ASHPlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 服务器：从 PlayerState 获取 ASC
	InitializeAbilitySystem();
}

void ASHPlayerCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 客户端：PlayerState 复制后初始化 ASC
	InitializeAbilitySystem();
}

void ASHPlayerCharacterBase::InitializeAbilitySystem()
{
	ASHPlayerState* PS = GetPlayerState<ASHPlayerState>();
	if (!PS)
	{
		return;
	}

	// 获取 ASC 和 AttributeSet 的缓存引用
	PlayerStateASC = PS->GetAbilitySystemComponent();
	PlayerAttributeSet = PS->GetPlayerAttributeSet();

	// 更新基类指针
	AttributeSet = PlayerAttributeSet;

	// 绑定死亡事件
	if (PlayerAttributeSet)
	{
		PlayerAttributeSet->OnDeath.AddUniqueDynamic(this, &ASHPlayerCharacterBase::OnDeathCallback);
	}

	// 设置 AvatarActor（确保能力使用正确的 Actor 位置和方向）
	PS->SetAvatarActor(this);
}

UAbilitySystemComponent* ASHPlayerCharacterBase::GetAbilitySystemComponent() const
{
	return PlayerStateASC;
}

USHPlayerAttributeSet* ASHPlayerCharacterBase::GetPlayerAttributeSet() const
{
	return PlayerAttributeSet;
}

float ASHPlayerCharacterBase::GetEnergy() const
{
	return PlayerAttributeSet ? PlayerAttributeSet->GetEnergy() : 0.f;
}

float ASHPlayerCharacterBase::GetMaxEnergy() const
{
	return PlayerAttributeSet ? PlayerAttributeSet->GetMaxEnergy() : 0.f;
}

bool ASHPlayerCharacterBase::CanJumpInternal_Implementation() const
{
	// 如果在 Coyote Time 内，直接允许跳跃（绕过 JumpCurrentCount 检查）
	if (CustomMovementComponent && CustomMovementComponent->IsInCoyoteTime())
	{
		return true;
	}

	return Super::CanJumpInternal_Implementation();
}

// ========== 搬运相关 ==========

void ASHPlayerCharacterBase::StartCarrying(const FBlockCarryState& InBlockState, UStaticMesh* InCarryMesh)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsCarrying = true;
	CarriedBlockState = InBlockState;
	CurrentCarryMesh = InCarryMesh;
	UpdateCarryVisual();
}

void ASHPlayerCharacterBase::StopCarrying()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsCarrying = false;
	CarriedBlockState = FBlockCarryState();
	CurrentCarryMesh = nullptr;
	UpdateCarryVisual();
}

void ASHPlayerCharacterBase::OnRep_CarryState()
{
	UpdateCarryVisual();
}

void ASHPlayerCharacterBase::UpdateCarryVisual()
{
	if (CarryMeshComponent)
	{
		CarryMeshComponent->SetVisibility(bIsCarrying);

		if (bIsCarrying && CurrentCarryMesh)
		{
			CarryMeshComponent->SetStaticMesh(CurrentCarryMesh);
			CarryMeshComponent->SetRelativeScale3D(FVector(CarryMeshScale));
		}
	}
}

// ========== 调试 ==========

void ASHPlayerCharacterBase::DrawDebugInfo()
{
	FVector Location = GetActorLocation() + FVector(0, 0, 200.f);

	// 获取技能列表
	FString AbilitiesText;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
		TArray<FGameplayAbilitySpec>& Abilities = ASC->GetActivatableAbilities();
		if (Abilities.Num() > 0)
		{
			AbilitiesText = TEXT("\nAbilities:");
			for (const FGameplayAbilitySpec& Spec : Abilities)
			{
				if (Spec.Ability)
				{
					AbilitiesText += FString::Printf(TEXT("\n  - %s"), *Spec.Ability->GetClass()->GetName());
				}
			}
		}
		else
		{
			AbilitiesText = TEXT("\nAbilities: None");
		}
	}

	FString DebugText = FString::Printf(
		TEXT("Player\nHP: %.0f/%.0f  Energy: %.0f/%.0f (+%.1f/s)\nATK: %.0f  DEF: %.0f%%\nCRIT: %.0f%%  CDMG: %.0f%%\nMSPD: %.0f%%%s"),
		GetHealth(),
		GetMaxHealth(),
		GetEnergy(),
		GetMaxEnergy(),
		PlayerAttributeSet ? PlayerAttributeSet->GetEnergyRegen() : 0.f,
		GetAttack(),
		GetDefence(),
		GetCriticalRate(),
		GetCriticalDamage(),
		PlayerAttributeSet ? PlayerAttributeSet->GetMovementSpeed() : 0.f,
		*AbilitiesText
	);

	DrawDebugString(GetWorld(), Location, DebugText, nullptr, FColor::Green, 0.f, false, 1.0f);
}
