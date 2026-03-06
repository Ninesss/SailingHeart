// Sailing Heart

#include "AbilitySystem/Ability/SHGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Data/Ability/SHAbilityDataBase.h"
#include "Data/Ability/SHAbilityParams.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "TimerManager.h"
#include "Engine/World.h"

USHGameplayAbilityBase::USHGameplayAbilityBase()
{
	// 默认实例化策略：每个 Actor 一个实例
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 默认网络执行策略：服务器发起
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// 默认安全策略：仅服务器执行
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyExecution;
}

// ========== 常用引用获取 ==========

UAbilitySystemComponent* USHGameplayAbilityBase::GetOwnerASC() const
{
	if (CurrentActorInfo)
	{
		return CurrentActorInfo->AbilitySystemComponent.Get();
	}
	return nullptr;
}

AActor* USHGameplayAbilityBase::GetSourceActor() const
{
	if (CurrentActorInfo)
	{
		return CurrentActorInfo->OwnerActor.Get();
	}
	return nullptr;
}

AActor* USHGameplayAbilityBase::GetAvatarActor() const
{
	if (CurrentActorInfo)
	{
		return CurrentActorInfo->AvatarActor.Get();
	}
	return nullptr;
}

AController* USHGameplayAbilityBase::GetOwnerController() const
{
	if (CurrentActorInfo)
	{
		if (APawn* Pawn = Cast<APawn>(CurrentActorInfo->AvatarActor.Get()))
		{
			return Pawn->GetController();
		}
	}
	return nullptr;
}

// ========== DataAsset 支持 ==========

USHAbilityDataBase* USHGameplayAbilityBase::GetAbilityData() const
{
	// 从 Spec.SourceObject 获取（授予能力时必须传入 DataAsset）
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (Spec && Spec->SourceObject.IsValid())
	{
		if (USHAbilityDataBase* Data = Cast<USHAbilityDataBase>(Spec->SourceObject.Get()))
		{
			return Data;
		}
	}

	return nullptr;
}

// ========== Cost系统 ==========

const USHAttributeSetBase* USHGameplayAbilityBase::GetBaseAttributeSet() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return nullptr;
	}
	return ASC->GetSet<USHAttributeSetBase>();
}

float USHGameplayAbilityBase::GetCurrentEnergy() const
{
	const USHAttributeSetBase* AttributeSet = GetBaseAttributeSet();
	if (!AttributeSet)
	{
		return 0.f;
	}
	return AttributeSet->GetEnergy();
}

bool USHGameplayAbilityBase::HasEnoughEnergy(float Amount) const
{
	if (Amount <= 0.f)
	{
		return true;
	}
	return GetCurrentEnergy() >= Amount;
}

void USHGameplayAbilityBase::SpendEnergy(float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	const USHAttributeSetBase* AttributeSet = GetBaseAttributeSet();
	if (!AttributeSet)
	{
		return;
	}

	float NewEnergy = FMath::Max(0.f, AttributeSet->GetEnergy() - Amount);
	ASC->SetNumericAttributeBase(USHAttributeSetBase::GetEnergyAttribute(), NewEnergy);
}

float USHGameplayAbilityBase::GetEnergyCost() const
{
	// 默认从DataAsset读取EnergyCost
	USHAbilityDataBase* Data = GetAbilityData();
	if (Data)
	{
		const FSHAbilityParams* Params = Data->GetBaseParams(GetAbilityLevel());
		if (Params)
		{
			return Params->EnergyCost;
		}
	}
	return 0.f;
}

// ========== Cooldown系统 ==========

FGameplayTag USHGameplayAbilityBase::GetAbilityCooldownTag() const
{
	// 从DataAsset读取CooldownTag
	USHAbilityDataBase* Data = GetAbilityData();
	if (Data)
	{
		const FSHAbilityParams* Params = Data->GetBaseParams(GetAbilityLevel());
		if (Params)
		{
			return Params->CooldownTag;
		}
	}
	return FGameplayTag();
}

float USHGameplayAbilityBase::GetAbilityCooldownDuration() const
{
	// 默认从DataAsset读取CooldownDuration
	USHAbilityDataBase* Data = GetAbilityData();
	if (Data)
	{
		const FSHAbilityParams* Params = Data->GetBaseParams(GetAbilityLevel());
		if (Params)
		{
			return Params->CooldownDuration;
		}
	}
	return 0.f;
}

bool USHGameplayAbilityBase::IsAbilityOnCooldown() const
{
	FGameplayTag CooldownTag = GetAbilityCooldownTag();
	if (!CooldownTag.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(CooldownTag);
}

void USHGameplayAbilityBase::ApplyAbilityCooldown()
{
	float Duration = GetAbilityCooldownDuration();
	if (Duration <= 0.f)
	{
		return;
	}

	FGameplayTag CooldownTag = GetAbilityCooldownTag();
	if (!CooldownTag.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 添加冷却Tag
	ASC->AddLooseGameplayTag(CooldownTag);

	// 设置定时器移除冷却Tag
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
	FGameplayTag TagToRemove = CooldownTag;

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(ASC, [WeakASC, TagToRemove]()
		{
			if (UAbilitySystemComponent* ASCLocal = WeakASC.Get())
			{
				ASCLocal->RemoveLooseGameplayTag(TagToRemove);
			}
		}),
		Duration,
		false
	);
}

// ========== 统一检查 ==========

bool USHGameplayAbilityBase::CheckAndCommitAbilityCost()
{
	// 检查冷却
	if (IsAbilityOnCooldown())
	{
		return false;
	}

	// 检查能量
	float Cost = GetEnergyCost();
	if (!HasEnoughEnergy(Cost))
	{
		return false;
	}

	// 消耗能量
	SpendEnergy(Cost);

	// 应用冷却
	ApplyAbilityCooldown();

	return true;
}
