// Sailing Heart

#include "Data/SHEnemyBlockData.h"

FBlockLevelConfig USHEnemyBlockData::GetLevelConfig(int32 Level) const
{
	if (const FBlockLevelConfig* Config = LevelConfigs.Find(Level))
	{
		return *Config;
	}

	// 回退到等级 1
	if (Level != 1)
	{
		if (const FBlockLevelConfig* Config = LevelConfigs.Find(1))
		{
			return *Config;
		}
	}

	// 返回默认值
	return FBlockLevelConfig();
}

int32 USHEnemyBlockData::GetMaxLevel() const
{
	int32 MaxLevel = 0;
	for (const auto& Pair : LevelConfigs)
	{
		MaxLevel = FMath::Max(MaxLevel, Pair.Key);
	}
	return MaxLevel;
}
