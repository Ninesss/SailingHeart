// Sailing Heart

#include "World/SHPostProcessVolumeBase.h"

ASHPostProcessVolumeBase::ASHPostProcessVolumeBase()
{
	// 无限范围（覆盖整个场景）
	bUnbound = true;

	FPostProcessSettings& PPSettings = Settings;

	// ========== 曝光 ==========
	// Metering Mode: Auto Exposure Basic
	PPSettings.bOverride_AutoExposureMethod = true;
	PPSettings.AutoExposureMethod = AEM_Basic;

	// Min EV100 = 1
	PPSettings.bOverride_AutoExposureMinBrightness = true;
	PPSettings.AutoExposureMinBrightness = 1.0f;

	// Max EV100 = 1
	PPSettings.bOverride_AutoExposureMaxBrightness = true;
	PPSettings.AutoExposureMaxBrightness = 1.0f;

	// 景深（DOF）由 PlayerCharacterBase 的 CameraComponent 动态管理，Volume 不覆盖

	// ========== 暗角 ==========
	// Vignette Intensity = 0
	PPSettings.bOverride_VignetteIntensity = true;
	PPSettings.VignetteIntensity = 0.0f;
}
