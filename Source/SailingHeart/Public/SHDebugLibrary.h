// Sailing Heart

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SHDebugLibrary.generated.h"

/**
 * 调试函数库 - 统一控制所有调试显示
 */
UCLASS()
class SAILINGHEART_API USHDebugLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 设置全局调试显示（方块 + 角色）
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void SetGlobalDebugDisplay(bool bShow);

	// 获取全局调试显示状态
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static bool GetGlobalDebugDisplay();

	// 切换全局调试显示
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void ToggleGlobalDebugDisplay();
};
