// Sailing Heart

#include "Character/SHEnemyCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Components/StateTreeComponent.h"
#include "Data/SHEnemyCharacterData.h"
#include "DrawDebugHelpers.h"

ASHEnemyCharacterBase::ASHEnemyCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 敌人角色使用基类的 ASC（自己拥有）

	// 创建 StateTree 组件
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void ASHEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASHEnemyCharacterBase::InitializeFromData(USHEnemyCharacterData* EnemyData)
{
	if (!EnemyData)
	{
		return;
	}

	EnemyCharacterData = EnemyData;
	CharacterTypeID = EnemyData->EnemyTypeID;

	// 初始化属性
	FCharacterInitParams Params;
	Params.MaxHealth = EnemyData->MaxHealth;
	Params.Attack = EnemyData->Attack;
	Params.Defence = EnemyData->Defence;
	Params.CriticalRate = EnemyData->CriticalRate;
	Params.CriticalDamage = EnemyData->CriticalDamage;
	InitializeCharacter(Params);
}

void ASHEnemyCharacterBase::DrawDebugInfo()
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
		TEXT("Enemy: %s\nHP: %.0f/%.0f\nATK: %.0f  DEF: %.0f%%\nCRIT: %.0f%%  CDMG: %.0f%%%s"),
		*CharacterTypeID.ToString(),
		GetHealth(),
		GetMaxHealth(),
		GetAttack(),
		GetDefence(),
		GetCriticalRate(),
		GetCriticalDamage(),
		*AbilitiesText
	);

	DrawDebugString(GetWorld(), Location, DebugText, nullptr, FColor::Red, 0.f, false, 1.0f);
}
