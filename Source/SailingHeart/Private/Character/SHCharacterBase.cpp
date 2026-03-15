// Sailing Heart

#include "Character/SHCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

// 静态变量定义
bool ASHCharacterBase::bGlobalShowDebugInfo = false;

ASHCharacterBase::ASHCharacterBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// 专用碎片推动胶囊：只 Block Debris channel，忽略所有游戏逻辑碰撞
	DebrisPushCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DebrisPushCapsule"));
	DebrisPushCapsule->SetupAttachment(GetRootComponent());
	DebrisPushCapsule->SetCollisionProfileName(TEXT("PhysicsMesh"));
	DebrisPushCapsule->SetCapsuleSize(34.f, 88.f);
}

void ASHCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHCharacterBase, CharacterTypeID);
}

void ASHCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASHCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGlobalShowDebugInfo)
	{
		DrawDebugInfo();
	}
}

void ASHCharacterBase::SetGlobalShowDebugAttributes(bool bShow)
{
	bGlobalShowDebugInfo = bShow;
}

void ASHCharacterBase::DrawDebugInfo()
{
	// 基类空实现，子类覆盖
}
