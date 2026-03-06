// Sailing Heart

#include "Player/SHPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/AttributeSet/SHPlayerAttributeSet.h"
#include "Data/SHPlayerCharacterData.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "Data/Ability/SHPassiveAbilityData.h"
#include "Game/SHGameStateBase.h"
#include "SHGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

// 静态空槽
FSHAbilitySlot ASHPlayerState::EmptySlot;

ASHPlayerState::ASHPlayerState()
{
	// 创建 ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed 模式：GE 只复制到 Owner，Cues 和 Tags 复制给所有人
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 创建属性集
	AttributeSet = CreateDefaultSubobject<USHPlayerAttributeSet>(TEXT("AttributeSet"));

	// 初始化能力槽
	AbilitySlots.SetNum(MAX_ABILITY_SLOTS);

	// 网络更新频率
	SetNetUpdateFrequency(100.f);
}

void ASHPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHPlayerState, SelectedCharacterID);
	DOREPLIFETIME(ASHPlayerState, AbilitySlots);
	DOREPLIFETIME(ASHPlayerState, CurrentSlotIndex);
	DOREPLIFETIME(ASHPlayerState, CurrentPassiveData);
}

void ASHPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// 初始化 ASC（OwnerActor = PlayerState, AvatarActor 暂时设为 this，会在 SetAvatarActor 中更新）
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 添加玩家阵营标签
		AbilitySystemComponent->AddLooseGameplayTag(FSHGameplayTags::Get().Faction_Player);
	}
}

void ASHPlayerState::ServerSelectCharacter_Implementation(FName CharacterID)
{
	// 验证 CharacterID 是否有效
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ASHGameStateBase* GS = World->GetGameState<ASHGameStateBase>();
	if (!GS)
	{
		return;
	}

	// 检查角色是否在可选列表中
	USHPlayerCharacterData* CharacterData = GS->GetPlayerCharacterData(CharacterID);
	if (CharacterData)
	{
		SelectedCharacterID = CharacterID;
	}
}

void ASHPlayerState::SetAvatarActor(AActor* NewAvatar)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 重新初始化 ActorInfo，设置正确的 AvatarActor
	AbilitySystemComponent->InitAbilityActorInfo(this, NewAvatar ? NewAvatar : this);
}

float ASHPlayerState::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float ASHPlayerState::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

float ASHPlayerState::GetAttack() const
{
	return AttributeSet ? AttributeSet->GetAttack() : 0.f;
}

float ASHPlayerState::GetDefence() const
{
	return AttributeSet ? AttributeSet->GetDefence() : 0.f;
}

float ASHPlayerState::GetEnergy() const
{
	return AttributeSet ? AttributeSet->GetEnergy() : 0.f;
}

float ASHPlayerState::GetMaxEnergy() const
{
	return AttributeSet ? AttributeSet->GetMaxEnergy() : 0.f;
}

void ASHPlayerState::InitializeFromCharacterData(USHPlayerCharacterData* CharacterData)
{
	if (!CharacterData || !AttributeSet)
	{
		return;
	}

	// 设置属性值
	AttributeSet->SetMaxHealth(CharacterData->MaxHealth);
	AttributeSet->SetHealth(CharacterData->MaxHealth);
	AttributeSet->SetAttack(CharacterData->Attack);
	AttributeSet->SetDefence(CharacterData->Defence);
	AttributeSet->SetCriticalRate(CharacterData->CriticalRate);
	AttributeSet->SetCriticalDamage(CharacterData->CriticalDamage);
	AttributeSet->SetMaxEnergy(CharacterData->MaxEnergy);
	AttributeSet->SetEnergy(CharacterData->MaxEnergy);
	AttributeSet->SetEnergyRegen(CharacterData->EnergyRegen);
	AttributeSet->SetMovementSpeed(CharacterData->MovementSpeed);

	// 装备初始主动能力（按数组下标对应槽位）
	for (int32 i = 0; i < CharacterData->InitialActiveAbilities.Num() && i < AbilitySlots.Num(); i++)
	{
		if (CharacterData->InitialActiveAbilities[i])
		{
			EquipAbility(CharacterData->InitialActiveAbilities[i], i);
		}
	}

	// 应用初始被动能力
	if (CharacterData->InitialPassiveAbility)
	{
		ApplyPassiveAbility(CharacterData->InitialPassiveAbility);
	}

	// 应用能量回复 GE（如果配置了且有能量系统）
	if (EnergyRegenEffectClass && AbilitySystemComponent && CharacterData->MaxEnergy > 0.f)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EnergyRegenEffectClass, 1, Context);
		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}
}

void ASHPlayerState::ClearAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->ClearAllAbilities();
}

// ========== 主动能力槽系统 ==========

const FSHAbilitySlot& ASHPlayerState::GetAbilitySlot(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < AbilitySlots.Num())
	{
		return AbilitySlots[SlotIndex];
	}
	return EmptySlot;
}

const FSHAbilitySlot& ASHPlayerState::GetCurrentAbilitySlot() const
{
	return GetAbilitySlot(CurrentSlotIndex);
}

bool ASHPlayerState::EquipAbility(USHAbilityDataBase* AbilityData, int32 SlotIndex)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!AbilityData || !AbilityData->AbilityClass)
	{
		return false;
	}

	if (SlotIndex < 0 || SlotIndex >= MAX_ABILITY_SLOTS)
	{
		return false;
	}

	if (!AbilitySystemComponent)
	{
		return false;
	}

	// 先卸下当前槽的能力
	UnequipAbility(SlotIndex);

	// 授予新能力，并传入 DataAsset 作为 SourceObject
	FGameplayAbilitySpec Spec(AbilityData->AbilityClass, 1);
	Spec.SourceObject = AbilityData;  // 能力可以通过 GetAbilityData() 获取配置
	FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);

	// 更新槽数据
	AbilitySlots[SlotIndex].AbilityData = AbilityData;
	AbilitySlots[SlotIndex].AbilitySpecHandle = Handle;

	return true;
}

void ASHPlayerState::UnequipAbility(int32 SlotIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (SlotIndex < 0 || SlotIndex >= AbilitySlots.Num())
	{
		return;
	}

	FSHAbilitySlot& Slot = AbilitySlots[SlotIndex];
	if (Slot.IsEmpty())
	{
		return;
	}

	// 从 ASC 移除能力
	if (AbilitySystemComponent && Slot.AbilitySpecHandle.IsValid())
	{
		AbilitySystemComponent->ClearAbility(Slot.AbilitySpecHandle);
	}

	// 清空槽
	Slot.Clear();
}

void ASHPlayerState::SwitchToNextSlot()
{
	int32 NextIndex = (CurrentSlotIndex + 1) % MAX_ABILITY_SLOTS;
	SwitchToSlot(NextIndex);
}

void ASHPlayerState::SwitchToSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MAX_ABILITY_SLOTS)
	{
		return;
	}

	if (CurrentSlotIndex != SlotIndex)
	{
		CurrentSlotIndex = SlotIndex;
	}
}

bool ASHPlayerState::TriggerCurrentAbility()
{
	const FSHAbilitySlot& CurrentSlot = GetCurrentAbilitySlot();
	if (CurrentSlot.IsEmpty())
	{
		return false;
	}

	if (!AbilitySystemComponent || !CurrentSlot.AbilitySpecHandle.IsValid())
	{
		return false;
	}

	// 确保 AvatarActor 是当前控制的 Pawn（安全检查）
	AController* OwningController = Cast<AController>(GetOwner());
	if (OwningController)
	{
		APawn* CurrentPawn = OwningController->GetPawn();
		if (CurrentPawn)
		{
			const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
			if (!ActorInfo || ActorInfo->AvatarActor.Get() != CurrentPawn)
			{
				SetAvatarActor(CurrentPawn);
			}
		}
	}

	// 直接尝试激活能力，Cost 和 Cooldown 由能力的 GE 自动处理
	return AbilitySystemComponent->TryActivateAbility(CurrentSlot.AbilitySpecHandle);
}

bool ASHPlayerState::CanTriggerCurrentAbility() const
{
	const FSHAbilitySlot& CurrentSlot = GetCurrentAbilitySlot();
	if (CurrentSlot.IsEmpty() || !CurrentSlot.AbilityData)
	{
		return false;
	}

	if (!AbilitySystemComponent || !CurrentSlot.AbilitySpecHandle.IsValid())
	{
		return false;
	}

	const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(CurrentSlot.AbilitySpecHandle);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	// 检查能力是否正在激活
	if (Spec->IsActive())
	{
		return false;
	}

	// 检查冷却（通过 Cooldown Tags）
	const FGameplayTagContainer* CooldownTags = Spec->Ability->GetCooldownTags();
	if (CooldownTags && CooldownTags->Num() > 0)
	{
		if (AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
		{
			return false;
		}
	}

	// Cost 检查由 TryActivateAbility 内部处理
	return true;
}

// ========== 被动能力 ==========

void ASHPlayerState::ApplyPassiveAbility(USHPassiveAbilityData* PassiveData)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!PassiveData || !PassiveData->PassiveEffectClass || !AbilitySystemComponent)
	{
		return;
	}

	// 先移除旧的被动效果
	RemovePassiveAbility();

	// 应用新的被动效果（Infinite Duration）
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		PassiveData->PassiveEffectClass, 1.f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		PassiveEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		CurrentPassiveData = PassiveData;
	}
}

void ASHPlayerState::RemovePassiveAbility()
{
	if (!HasAuthority())
	{
		return;
	}

	if (PassiveEffectHandle.IsValid() && AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(PassiveEffectHandle);
	}

	PassiveEffectHandle = FActiveGameplayEffectHandle();
	CurrentPassiveData = nullptr;
}

// ========== 复制回调 ==========

void ASHPlayerState::OnRep_AbilitySlots()
{
}

void ASHPlayerState::OnRep_CurrentSlotIndex()
{
}
