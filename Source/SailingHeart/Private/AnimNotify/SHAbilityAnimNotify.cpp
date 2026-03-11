// Sailing Heart

#include "AnimNotify/SHAbilityAnimNotify.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Interface/SHCombatInterface.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

void USHAbilityAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// 只在服务端处理（AI 单位都是服务端权威，不需要客户端预测）
	if (!Owner->HasAuthority())
	{
		return;
	}

	// 通过 ISHCombatInterface 获取对应 Socket 的世界变换
	FTransform SocketTransform = FTransform::Identity;
	if (Owner->Implements<USHCombatInterface>())
	{
		SocketTransform = ISHCombatInterface::Execute_GetCombatSocketTransform(Owner, EventTag);
	}
	else if (MeshComp->DoesSocketExist(FallbackSocketName))
	{
		SocketTransform = MeshComp->GetSocketTransform(FallbackSocketName, RTS_World);
	}
	else
	{
		SocketTransform = Owner->GetActorTransform();
	}

	// 将 Socket 位置封装为 TargetData，供 Ability 读取发射点
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
	LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData->TargetLocation.LiteralTransform = SocketTransform;

	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(LocationData);

	// 构造 Event Payload
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Owner;
	Payload.Target = Owner;
	Payload.TargetData = DataHandle;

	// 向 Actor 的 ASC 广播事件，Ability 通过 WaitGameplayEvent 接收
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
}
