// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "SHPostProcessVolumeBase.generated.h"

/**
 * 项目基础后处理体积
 * 预设：Auto Exposure Basic / EV100=1 / f/3 / 无暗角 / 传感器宽度600 / 焦距2550 / 无限范围
 */
UCLASS()
class SAILINGHEART_API ASHPostProcessVolumeBase : public APostProcessVolume
{
	GENERATED_BODY()

public:
	ASHPostProcessVolumeBase();
};
