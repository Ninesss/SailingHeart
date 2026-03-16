// Sailing Heart


#include "Game/SHGameStateBase.h"
#include "Grid/SHGridBase.h"
#include "Data/SHPlayerBlockData.h"
#include "Data/SHPlayerCharacterData.h"
#include "Net/UnrealNetwork.h"
#include "Physics/Experimental/PhysScene_Chaos.h"

ASHGameStateBase::ASHGameStateBase()
{
	bReplicates = true;
}

void ASHGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHGameStateBase, GlobalTimeScale);
}

void ASHGameStateBase::SetGlobalTimeScale(float NewTimeScale)
{
	if (!HasAuthority())
	{
		return;
	}

	const float OldTimeScale = GlobalTimeScale;
	GlobalTimeScale = FMath::Clamp(NewTimeScale, 0.0f, 10.0f);

	// 只在值变化时广播（服务器端，OnRep 不会在服务器触发）
	if (OldTimeScale != GlobalTimeScale)
	{
		BroadcastTimeScaleChanged();
	}
}

void ASHGameStateBase::OnRep_GlobalTimeScale()
{
	// 客户端收到同步后广播
	BroadcastTimeScaleChanged();
}

void ASHGameStateBase::RegisterActorForTimeScale(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	TimeScaleActors.AddUnique(Actor);
	Actor->CustomTimeDilation = GlobalTimeScale;
}

void ASHGameStateBase::BroadcastTimeScaleChanged()
{
	// 同步 Chaos Solver 时间缩放（影响 GC 碎片物理）
	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{
			PhysScene->SetNetworkDeltaTimeScale(GlobalTimeScale);
		}
	}

	// 统一设置所有注册 Actor 的 CustomTimeDilation
	TimeScaleActors.RemoveAll([](const TWeakObjectPtr<AActor>& A) { return !A.IsValid(); });
	for (const TWeakObjectPtr<AActor>& WeakActor : TimeScaleActors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->CustomTimeDilation = GlobalTimeScale;
		}
	}

	OnGlobalTimeScaleChanged.Broadcast(GlobalTimeScale);
}

void ASHGameStateBase::ActivateTimeSlow()
{
	if (!HasAuthority())
	{
		return;
	}

	TimeSlowActiveCount++;

	// 从 0 变为 1 时，启用减速
	if (TimeSlowActiveCount == 1)
	{
		SetGlobalTimeScale(SlowedTimeScale);
	}
}

void ASHGameStateBase::DeactivateTimeSlow()
{
	if (!HasAuthority())
	{
		return;
	}

	TimeSlowActiveCount = FMath::Max(0, TimeSlowActiveCount - 1);

	// 变为 0 时，恢复正常速度
	if (TimeSlowActiveCount == 0)
	{
		SetGlobalTimeScale(1.0f);
	}
}

void ASHGameStateBase::RegisterPlayerGrid(ASHGridBase* Grid)
{
	if (HasAuthority() && Grid)
	{
		PlayerGrid = Grid;
	}
}

USHPlayerBlockData* ASHGameStateBase::GetPlayerBlockData(FName BlockTypeID) const
{
	for (int32 i = 0; i < PlayerBlockDataList.Num(); i++)
	{
		USHPlayerBlockData* Data = PlayerBlockDataList[i];
		if (Data)
		{
			if (Data->BlockTypeID == BlockTypeID)
			{
				return Data;
			}
		}
	}

	return nullptr;
}

USHPlayerCharacterData* ASHGameStateBase::GetPlayerCharacterData(FName CharacterID) const
{
	for (USHPlayerCharacterData* Data : PlayerCharacterDataList)
	{
		if (Data && Data->CharacterID == CharacterID)
		{
			return Data;
		}
	}
	return nullptr;
}

TArray<FName> ASHGameStateBase::GetAvailableCharacterIDs() const
{
	TArray<FName> IDs;
	for (USHPlayerCharacterData* Data : PlayerCharacterDataList)
	{
		if (Data)
		{
			IDs.Add(Data->CharacterID);
		}
	}
	return IDs;
}
