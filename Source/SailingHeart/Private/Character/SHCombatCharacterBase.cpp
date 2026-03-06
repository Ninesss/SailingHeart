// Sailing Heart

#include "Character/SHCombatCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "DrawDebugHelpers.h"

ASHCombatCharacterBase::ASHCombatCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 创建 AbilitySystemComponent
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 默认使用基础属性集
	AttributeSetClass = USHAttributeSetBase::StaticClass();
}

void ASHCombatCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 创建属性集
	if (AbilitySystemComponent && AttributeSetClass)
	{
		AttributeSet = NewObject<USHAttributeSetBase>(this, AttributeSetClass);
		AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
	}

	// 初始化 ASC
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 绑定死亡事件
	if (AttributeSet)
	{
		AttributeSet->OnDeath.AddDynamic(this, &ASHCombatCharacterBase::OnDeathCallback);
	}
}

UAbilitySystemComponent* ASHCombatCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// ========== 属性访问器 ==========

float ASHCombatCharacterBase::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float ASHCombatCharacterBase::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

float ASHCombatCharacterBase::GetAttack() const
{
	return AttributeSet ? AttributeSet->GetAttack() : 0.f;
}

float ASHCombatCharacterBase::GetDefence() const
{
	return AttributeSet ? AttributeSet->GetDefence() : 0.f;
}

// ========== ISHCombatInterface 实现 ==========

bool ASHCombatCharacterBase::IsDead_Implementation() const
{
	return GetHealth() <= 0.f;
}

void ASHCombatCharacterBase::Die_Implementation()
{
	HandleDeath();
}

AActor* ASHCombatCharacterBase::GetAvatar_Implementation()
{
	return this;
}

float ASHCombatCharacterBase::GetCriticalRate() const
{
	return AttributeSet ? AttributeSet->GetCriticalRate() : 0.f;
}

float ASHCombatCharacterBase::GetCriticalDamage() const
{
	return AttributeSet ? AttributeSet->GetCriticalDamage() : 200.f;
}

// ========== 初始化 ==========

void ASHCombatCharacterBase::InitializeCharacter(const FCharacterInitParams& Params)
{
	InitializeAttributes(Params);
}

void ASHCombatCharacterBase::InitializeAttributes(const FCharacterInitParams& Params)
{
	if (!AttributeSet)
	{
		return;
	}

	AttributeSet->SetMaxHealth(Params.MaxHealth);
	AttributeSet->SetAttack(Params.Attack);
	AttributeSet->SetDefence(Params.Defence);
	AttributeSet->SetCriticalRate(Params.CriticalRate);
	AttributeSet->SetCriticalDamage(Params.CriticalDamage);

	// 设置当前血量
	const float Health = (Params.CurrentHealth < 0.f) ? Params.MaxHealth : Params.CurrentHealth;
	AttributeSet->SetHealth(Health);
}

// ========== 死亡处理 ==========

void ASHCombatCharacterBase::OnDeathCallback(AActor* DeadActor)
{
	if (DeadActor == this)
	{
		HandleDeath();
	}
}

void ASHCombatCharacterBase::HandleDeath_Implementation()
{
	// 防止重复调用
	if (bIsDying)
	{
		return;
	}
	bIsDying = true;

	// 基类默认实现：销毁（子类可覆盖）
	Destroy();
}

// ========== 调试 ==========

void ASHCombatCharacterBase::DrawDebugInfo()
{
	FVector Location = GetActorLocation() + FVector(0, 0, 200.f);

	FString DebugText = FString::Printf(
		TEXT("Character\nHP: %.0f/%.0f\nATK: %.0f  DEF: %.0f%%\nCRIT: %.0f%%  CDMG: %.0f%%"),
		GetHealth(),
		GetMaxHealth(),
		GetAttack(),
		GetDefence(),
		GetCriticalRate(),
		GetCriticalDamage()
	);

	DrawDebugString(GetWorld(), Location, DebugText, nullptr, FColor::White, 0.f, false, 1.0f);
}
