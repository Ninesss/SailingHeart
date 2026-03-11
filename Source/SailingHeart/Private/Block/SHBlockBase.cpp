// Sailing Heart

#include "Block/SHBlockBase.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

// 静态变量定义
bool ASHBlockBase::bGlobalShowDebugInfo = false;

ASHBlockBase::ASHBlockBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// 启用网络复制
	bReplicates = true;
	bAlwaysRelevant = true;

	// 创建根组件
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// 创建 Mesh 组件（静态底座）
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComponent->SetVisibleInRayTracing(false);

	// 创建功能 SKM（用于播放动画，SKM 资产在蓝图子类中配置）
	FunctionalSKM = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FunctionalSKM"));
	FunctionalSKM->SetupAttachment(Root);
	FunctionalSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FunctionalSKM->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FunctionalSKM->SetVisibleInRayTracing(false);
	FunctionalSKM->SetIsReplicated(true);

	// 创建碰撞盒（默认禁用，子类启用）
	BlockCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockCollisionBox"));
	BlockCollisionBox->SetupAttachment(Root);
	BlockCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockCollisionBox->SetGenerateOverlapEvents(true);

	// 创建 StateTree 组件
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void ASHBlockBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHBlockBase, BlockTypeID);
}

void ASHBlockBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASHBlockBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGlobalShowDebugInfo)
	{
		DrawDebugInfo();
	}
}

void ASHBlockBase::SetGlobalShowDebugAttributes(bool bShow)
{
	bGlobalShowDebugInfo = bShow;
}

void ASHBlockBase::SetupBlockCollision()
{
	if (!BlockCollisionBox)
	{
		return;
	}

	// 设置碰撞盒大小（高度为宽度两倍）
	const float HalfSize = BlockCellSize * CollisionSizeRatio * 0.5f;
	BlockCollisionBox->SetBoxExtent(FVector(HalfSize, HalfSize, HalfSize * 2.f));

	// 碰撞盒位置（Z 轴偏移使底部与方块底部对齐）
	BlockCollisionBox->SetRelativeLocation(FVector(0.f, 0.f, HalfSize * 2.f));

	// 注意：不在这里设置 Profile，因为 Profile 会启用碰撞
	// 碰撞在 EnableBlockCollision() 中启用
}

void ASHBlockBase::EnableBlockCollision()
{
	if (!BlockCollisionBox)
	{
		return;
	}

	// 设置碰撞预设
	BlockCollisionBox->SetCollisionProfileName(BlockCollisionProfileName);
}

FString ASHBlockBase::GetStateTreeDebugText() const
{
	if (!StateTreeComponent)
	{
		return TEXT("ST: No Component");
	}

	if (StateTreeComponent->IsRunning())
	{
		return TEXT("ST: Running");
	}
	else
	{
		return TEXT("ST: Stopped");
	}
}
