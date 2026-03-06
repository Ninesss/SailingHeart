// Sailing Heart

#include "AbilitySystem/SHDamageExecCalc.h"
#include "AbilitySystem/AttributeSet/SHAttributeSetBase.h"
#include "AbilitySystem/SHGameplayEffectContext.h"
#include "AbilitySystemComponent.h"
#include "SHGameplayTags.h"

// 属性捕获定义
struct SHDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defence);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);

	SHDamageStatics()
	{
		// 攻击力：来自源（伤害输出者），快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(USHAttributeSetBase, Attack, Source, true);

		// 防御力：来自目标（受伤害者），不快照（实时值）
		DEFINE_ATTRIBUTE_CAPTUREDEF(USHAttributeSetBase, Defence, Target, false);

		// 血量：来自源（用于碰撞伤害），快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(USHAttributeSetBase, Health, Source, true);

		// 暴击率：来自源，快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(USHAttributeSetBase, CriticalRate, Source, true);

		// 暴击伤害：来自源，快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(USHAttributeSetBase, CriticalDamage, Source, true);
	}
};

static const SHDamageStatics& DamageStatics()
{
	static SHDamageStatics Statics;
	return Statics;
}

USHDamageExecCalc::USHDamageExecCalc()
{
	// 注册需要捕获的属性
	RelevantAttributesToCapture.Add(DamageStatics().AttackDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenceDef);
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalRateDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageDef);
}

void USHDamageExecCalc::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

	// 获取自定义 Context
	FSHGameplayEffectContext* SHContext = static_cast<FSHGameplayEffectContext*>(ContextHandle.Get());

	// 设置评估参数
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = SourceTags;
	EvaluateParams.TargetTags = TargetTags;

	float FinalDamage = 0.f;
	bool bIsCriticalHit = false;

	// 根据伤害类型选择计算方式
	if (SHContext && SHContext->IsCollisionDamage())
	{
		// ========== 碰撞伤害 ==========
		// 直接使用 Context 中存储的伤害值（对方剩余血量）
		FinalDamage = SHContext->GetCollisionDamageOverride();
	}
	else
	{
		// ========== 普通伤害（子弹/技能）==========
		// 捕获属性值
		float SourceAttack = 0.f;
		float TargetDefence = 0.f;
		float SourceCritRate = 0.f;
		float SourceCritDamage = 150.f;  // 默认 150%

		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
			DamageStatics().AttackDef, EvaluateParams, SourceAttack);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
			DamageStatics().DefenceDef, EvaluateParams, TargetDefence);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
			DamageStatics().CriticalRateDef, EvaluateParams, SourceCritRate);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
			DamageStatics().CriticalDamageDef, EvaluateParams, SourceCritDamage);

		// 数值范围限制
		SourceAttack = FMath::Max(0.f, SourceAttack);
		TargetDefence = FMath::Clamp(TargetDefence, 0.f, 100.f);
		SourceCritRate = FMath::Clamp(SourceCritRate, 0.f, 100.f);
		SourceCritDamage = FMath::Max(SourceCritDamage, 100.f);

		// 获取伤害倍率（SetByCaller，默认 1.0）
		float DamageMultiplier = Spec.GetSetByCallerMagnitude(
			FSHGameplayTags::Get().SetByCaller_Damage_Multiplier,
			false,  // 不警告找不到
			1.0f    // 默认值
		);
		DamageMultiplier = FMath::Max(0.f, DamageMultiplier);

		// 伤害公式: Damage = Attack * DamageMultiplier * (1 - Defence/100)
		const float DefenceMultiplier = 1.f - (TargetDefence / 100.f);
		FinalDamage = SourceAttack * DamageMultiplier * DefenceMultiplier;

		// 暴击判定（根据 CriticalRate 随机判定）
		const float CritRoll = FMath::FRandRange(0.f, 100.f);
		bIsCriticalHit = CritRoll < SourceCritRate;

		if (bIsCriticalHit)
		{
			// 暴击伤害倍率
			FinalDamage *= (SourceCritDamage / 100.f);

			// 标记到 Context 供后续使用（如显示暴击特效）
			if (SHContext)
			{
				SHContext->SetIsCriticalHit(true);
			}
		}
	}

	// 确保伤害不为负
	FinalDamage = FMath::Max(0.f, FinalDamage);

	// 输出到 IncomingDamage 元属性
	if (FinalDamage > 0.f)
	{
		FGameplayModifierEvaluatedData EvaluatedData(
			USHAttributeSetBase::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive,
			FinalDamage);
		OutExecutionOutput.AddOutputModifier(EvaluatedData);
	}
}
