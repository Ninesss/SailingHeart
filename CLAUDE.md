# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SailingHeart is a multiplayer-networked action game built on Unreal Engine 5 using C++ and GAS (Gameplay Ability System). It features a grid-based block combat system where players place and carry blocks that fight enemy blocks. The UI and comments are in Chinese.

## Build System

This is a standard UE5 project. There are no custom build scripts — use Unreal Engine tools:

- **Generate project files:** Right-click `SailingHeart.uproject` → "Generate Visual Studio project files"
- **Build:** Open `SailingHeart.sln` in Visual Studio and build the `SailingHeart` target, or use the UE Editor's built-in compilation
- **Hot reload:** In the Editor, use the "Compile" button (Ctrl+Alt+F11) for live C++ recompilation
- **UnrealBuildTool (CLI):** `UnrealBuildTool SailingHeart Win64 Development -Project="<path>/SailingHeart.uproject"`

Module dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `ProceduralMeshComponent`, `GameplayAbilities`, `GameplayTags`, `GameplayTasks`, `Niagara`, `StateTreeModule`, `GameplayStateTreeModule`.

Key plugins: `GameplayAbilities`, `GameplayStateTree`, `PaperZD`.

## Code Architecture

### Single Module
All C++ lives in one module: `SailingHeart` (Runtime). Public headers in `Source/SailingHeart/Public/`, implementations in `Source/SailingHeart/Private/`.

### Core Systems

**Game Framework**
- `ASHGameModeBase` — Player spawn logic, resolves character DataAssets per controller.
- `ASHGameStateBase` — Central authority for: player grid reference, block/character DataAsset registries, and the **global time scale system** (replicated `GlobalTimeScale` + reference-counted `TimeSlowActiveCount`).
- `ASHPlayerController` — Enhanced Input binding; all gameplay actions go through Server RPCs (`ServerPlaceBlock`, `ServerCarryBlock`, `ServerTriggerAbility`, `ServerSwitchAbilitySlot`).
- `ASHPlayerState` — Owns the player's `UAbilitySystemComponent` (persists across respawns per GAS best practices). Manages a 2-slot ability system (`FSHAbilitySlot`) and character initialization.

**Character Hierarchy**
```
ACharacter
  └─ ASHCharacterBase              (CharacterTypeID, debug)
       └─ ASHCombatCharacterBase   (GAS integration, ISHCombatInterface)
            ├─ ASHPlayerCharacterBase  (uses PlayerState's ASC, carry mechanic, camera)
            └─ ASHEnemyCharacterBase   (owns its own ASC, StateTree AI)
```
Custom movement: `USHCharacterMovementComponent` adds flat-base floor detection, **Coyote Time** (~0.15s), and asymmetric gravity (`RisingGravityScale` / `FallingGravityScale`).

**Block System**
```
AActor
  └─ ASHBlockBase              (StaticMesh, BoxCollision, StateTreeComponent, FactionTag, BlockTypeID)
       └─ ASHCombatBlockBase   (GAS, ISHCombatInterface; InitializeBlock(FBlockInitParams))
            ├─ ASHPlayerBlock  (cell position on grid, carry/merge/upgrade, collision damage)
            └─ ASHEnemyBlock   (spawned from USHEnemyBlockData)
  └─ ASHNeutralBlock           (no GAS; converts to ASHPlayerBlock on grid overlap)
```
All block types use `SpawnActorDeferred` / `FinishSpawning` via static `SpawnDeferred(...)` factory methods. `FBlockLevelConfig` holds per-level stats (Health, Energy, Attack, Defence, Crit, ActionSpeed, GrantedAbilities).

**Grid System**
- `ASHGridBase` — `UProceduralMeshComponent` renders grid lines. `TMap<FIntPoint, ASHPlayerBlock*> CellData` (server-side). `TMap<FIntPoint, UBoxComponent*> CellCollisions` for overlap. Replicates `OccupiedCells` for late-join sync.
- `ASHWorldBlockSpawner` — Spawns waves of enemy + neutral blocks ahead of the grid on a timer.
- `USHGridFunctionLibrary` — Blueprint library for global grid-coordinate math (GridToWorld, WorldToGrid, SnapToGrid). Default `CellSize = 200 UU`; grid coords are global, not per-actor.

**GAS (Gameplay Ability System)**

AttributeSet hierarchy:
```
USHAttributeSetBase      (Health, Attack, Defence, Crit, Energy/EnergyRegen, IncomingDamage meta, OnDeath delegate)
  ├─ USHBlockAttributeSet    (+ Level, ActionSpeed)
  └─ USHPlayerAttributeSet   (+ MovementSpeed)
```

Ability hierarchy:
```
USHGameplayAbilityBase           (energy cost, cooldown via GameplayTag+Timer, DataAsset via SourceObject)
  ├─ USHGameplayAbility_Projectile   (fires 1-N projectiles; Linear/Homing/Parabolic)
  └─ USHGameplayAbility_Toggle       (toggle on/off, continuous energy drain)
       └─ USHGameplayAbility_TimeSlow  (sets GlobalTimeScale via GameState)
```

Custom GE context: `FSHGameplayEffectContext` extends `FGameplayEffectContext` with `DamageTypeTags`, `bIsCriticalHit`, `CollisionDamageOverride` (full `NetSerialize`).

Damage execution (`USHDamageExecCalc`): two branches — **Physical** (Attack/Defence + crit roll) vs **Collision** (uses `CollisionDamageOverride` directly, bypasses Attack/Defence).

**DataAsset-Driven Configuration**

All entities are configured through `UPrimaryDataAsset` subclasses. Ability parameters are passed as `SourceObject` on `FGameplayAbilitySpec`. Key DataAsset types:
- `USHPlayerBlockData`, `USHEnemyBlockData`, `USHNeutralBlockData` — block configs
- `USHPlayerCharacterData`, `USHEnemyCharacterData` — character configs
- `USHProjectileAbilityData`, `USHTimeSlowAbilityData`, `USHPassiveAbilityData` — ability configs

**GameplayTags**

All native tags defined in `FSHGameplayTags` (`SHGameplayTags.h`) and initialized in `USHAssetManager::StartInitialLoading()`. Key tag namespaces:
- `DamageType.Physical` / `DamageType.Collision`
- `Faction.Player` / `Faction.Enemy` / `Faction.Neutral`
- `Attribute.Vital.*` / `Attribute.Combat.*` / `Attribute.Block.*` / `Attribute.Player.*`
- `ToggleState.Player.TimeSlow`
- `Cooldown.Player.Projectile.*`

**Projectile System**
- `ASHProjectileBase` — `USphereComponent` + `USHProjectileMovementComponent` + `UNiagaraComponent`. Supports penetration, XY-plane homing with auto-retarget on target death. Subscribes to `OnGlobalTimeScaleChanged` to scale with time-slow.
- `USHProjectileMovementComponent` — Overrides `ComputeHomingAcceleration` for XY-only tracking; scales `DeltaTime` by `GlobalTimeScale` when `bUseGlobalTimeScale = true`.

### Key Architectural Patterns

1. **ASC placement:** Player ASC lives on `ASHPlayerState` (survives respawn); enemy/block ASCs live on the actor itself.
2. **Unified block init:** `ASHCombatBlockBase::InitializeBlock(FBlockInitParams)` is the single entry point — never partially initialize a block.
3. **Time-slow is reference-counted:** `GameState->TimeSlowActiveCount` prevents conflicts in multiplayer; `GlobalTimeScale` is replicated via `OnRep_GlobalTimeScale`.
4. **Faction system:** Use `USHAbilitySystemLibrary::GetActorFaction()`, `AreActorsFriends()`, `AreActorsEnemies()` for faction checks — do not query tags directly.
5. **StateTree AI:** Both blocks (`ASHBlockBase`) and enemy characters (`ASHEnemyCharacterBase`) use `UStateTreeComponent` from the `GameplayStateTree` plugin.
6. **Shared structs:** `FGridCell` and `FBlockCarryState` (in `Save/SHBlockTypes.h`) are used across Grid, PlayerBlock, and PlayerCharacter — avoid redefining these.
