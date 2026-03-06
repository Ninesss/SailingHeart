// Sailing Heart

#include "Data/SHPlayerBlockData.h"

FBlockLevelConfig USHPlayerBlockData::GetLevelConfig(int32 Level) const
{
	if (const FBlockLevelConfig* Config = LevelConfigs.Find(Level))
	{
		return *Config;
	}

	// 返回默认值
	return FBlockLevelConfig();
}

int32 USHPlayerBlockData::GetMaxLevel() const
{
	int32 MaxLevel = 0;
	for (const auto& Pair : LevelConfigs)
	{
		MaxLevel = FMath::Max(MaxLevel, Pair.Key);
	}
	return MaxLevel;
}
