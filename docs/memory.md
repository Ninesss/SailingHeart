# SailingHeart — Project Memory

> Auto-generated reference document. Last updated: 2026-03-29.

---

## Project Overview

SailingHeart is a multiplayer-networked action game built on **Unreal Engine 5** using **C++** and **GAS (Gameplay Ability System)**. Players place and carry blocks on a moving grid to fight incoming enemy blocks. All UI labels and code comments are in **Chinese**.

**Key traits:**
- Single C++ runtime module: `SailingHeart`
- Multiplayer via UE replication (Server RPCs + `Replicated` properties)
- DataAsset-driven entity configuration
- StateTree AI for both blocks and enemy characters
- GAS for all combat (abilities, attributes, effects, execution calculations)

**Plugins:** `GameplayAbilities`, `GameplayStateTree`, `PaperZD`, `MotionWarping` (partially integrated), `ChaosClothAsset`

---

## Architecture

### Game Framework

| Class | Responsibility |
|---|---|
| `ASHGameStateBase` | Central authority: `GlobalTimeScale` (replicated, reference-counted), grid registry, block/character DataAsset registries |
| `ASHGameModeBase` | Player spawn — resolves character DataAsset per controller, calls `InitializePlayerCharacter()` |
| `ASHPlayerState` | Owns player's `UAbilitySystemComponent` (persists across respawns). Manages 2-slot ability system (`FSHAbilitySlot`) + passive ability |
| `ASHPlayerController` | Enhanced Input bindings → Server RPCs: `ServerPlaceBlock`, `ServerCarryBlock`, `ServerTriggerAbility`, `ServerSwitchAbilitySlot`, `ServerSelectCharacter` |

### Character Hierarchy

```
ACharacter
  └─ ASHCharacterBase              (CharacterTypeID, DebrisPushCapsule)
       └─ ASHCombatCharacterBase   (GAS, ISHCombatInterface, AbilityMontageMap)
            ├─ ASHPlayerCharacterBase  (uses PlayerState ASC, carry mechanic, camera)
            └─ ASHEnemyCharacterBase   (owns ASC, StateTree AI, ISHAIEntityInterface)
```

**`USHCharacterMovementComponent`** adds:
- Flat-base floor detection (`FlatBaseBoxRatio`)
- Coyote Time (`CoyoteTimeDuration` ~0.15s)
- Asymmetric gravity (`RisingGravityScale` / `FallingGravityScale`)

### Block Hierarchy

```
AActor
  └─ ASHBlockBase              (StaticMesh, FunctionalSKM, BoxCollision, StateTree, FactionTag, BlockTypeID)
       └─ ASHCombatBlockBase   (GAS, ISHCombatInterface; InitializeBlock(FBlockInitParams))
            ├─ ASHPlayerBlock  (Faction.Ally; grid cell, carry/merge/upgrade, collision damage)
            └─ ASHEnemyBlock   (Faction.Enemy; spawned from USHEnemyBlockData)
  └─ ASHNeutralBlock           (no GAS; converts to ASHPlayerBlock on grid overlap)
```

All blocks use `SpawnActorDeferred` / `FinishSpawning` via static `SpawnDeferred(...)` factory methods.

`FBlockLevelConfig` — per-level stats: Health, Energy, EnergyRegen, Attack, Defence, Crit, ActionSpeed, GrantedAbilities.

### Grid System

| Class | Responsibility |
|---|---|
| `ASHGridBase` | `UProceduralMeshComponent` renders lines. `TMap<FIntPoint, ASHPlayerBlock*> CellData` (server-side). Replicates `OccupiedCells` for late-join sync. Supports directional movement (`bEnableMovement`, `MovementSpeed`) |
| `ASHWorldBlockSpawner` | Timed waves of enemy + neutral blocks ahead of the grid |
| `USHGridFunctionLibrary` | Blueprint library: `GridToWorld`, `WorldToGrid`, `SnapToGrid`. Default `CellSize = 200 UU`. Coords are **global**, not per-actor |

### GAS — Attribute Hierarchy

```
USHAttributeSetBase      (Health, Attack, Defence, Crit, Energy/EnergyRegen, IncomingDamage meta, OnDeath delegate)
  ├─ USHBlockAttributeSet    (+ Level, ActionSpeed)
  └─ USHPlayerAttributeSet   (+ MovementSpeed)
```

### GAS — Ability Hierarchy

```
USHGameplayAbilityBase                  (energy cost, cooldown via Tag+Timer, DataAsset via SourceObject)
  ├─ USHGameplayAbility_Projectile      (fires 1–N projectiles; Linear/Homing/Parabolic; two-stage: Montage→Event→Fire)
  ├─ USHGameplayAbility_Melee           (melee attacks)
  └─ USHGameplayAbility_Toggle          (toggle on/off, continuous energy drain)
       └─ USHGameplayAbility_TimeSlow   (sets GlobalTimeScale via GameState)
```

### Damage Flow

`USHDamageExecCalc` has two branches:
- **Physical** — `Attacker.Attack × (1 − Defender.Defence%)` + crit roll (using `CriticalRate`, `CriticalDamage`)
- **Collision** — uses `FSHGameplayEffectContext.CollisionDamageOverride` directly (bypasses Attack/Defence)

Custom context `FSHGameplayEffectContext` extends `FGameplayEffectContext` with:
`DamageTypeTags`, `bIsCriticalHit`, `CollisionDamageOverride`, `HitDirection`

### Projectile System

- `ASHProjectileBase` — `USphereComponent` + `USHProjectileMovementComponent` + `UNiagaraComponent`
  - Penetration support (`MaxPenetrations`), XY-plane homing, auto-retarget on target death
  - Listens to `OnGlobalTimeScaleChanged` to scale with time-slow
- `USHProjectileMovementComponent` — overrides `ComputeHomingAcceleration` for XY-only tracking; scales `DeltaTime` by `GlobalTimeScale` when `bUseGlobalTimeScale = true`

### AI System (StateTree)

All AI runs via `UStateTreeComponent` — no traditional AI controllers.

| Component | Role |
|---|---|
| `ISHAIEntityInterface` | Single method `GetAIConfig()` → `FSHAIConfig`. Implemented by `ASHPlayerBlock`, `ASHEnemyBlock`, `ASHEnemyCharacterBase` |
| `FSHAIConfig` | `DetectionRange`, `AttackRange`, `bCanMove`. Embedded in DataAssets |
| `FSHAIEvaluator` | Tick evaluator — sphere detection, outputs `CurrentTarget`, `bInDetectionRange`, `bInAttackRange` |
| `FSHAIUseAbilityTask` | Cycles through `AbilityEventTags` list, fires each via `SendGameplayEvent` |
| `FSHAIMoveToTargetTask` | Movement toward target |
| `FSHAIWatchTargetTask` | Look-at rotation |
| `FSHAIConditions` | Decision conditions for StateTree transitions |

StateTree module path: `Public/AI/StateTree/`

### DataAsset System

| DataAsset | Configures |
|---|---|
| `USHPlayerBlockData` | `BlockTypeID`, `BlockClass`, `TMap<int32, FBlockLevelConfig> LevelConfigs`, `FSHAIConfig` |
| `USHEnemyBlockData` | Same pattern, `EnemyBlock` class |
| `USHNeutralBlockData` | Neutral block class + mesh |
| `USHPlayerCharacterData` | Character class, initial attributes, abilities |
| `USHEnemyCharacterData` | Character class + `FSHAIConfig` |
| `USHProjectileAbilityData` | Projectile class, speed, fire count, spread, homing params |
| `USHTimeSlowAbilityData` | Time scale factor, duration |
| `USHPassiveAbilityData` | Passive GE class + params |

Ability parameters are passed as `Spec.SourceObject` — abilities call `GetAbilityData<T>()` to retrieve config.

### Animation & Combat Integration

**Two-stage ability execution** (Projectile):
1. Play montage on avatar via `PlayMontageAndWaitForEvent(TriggerTag)`
2. `USHAbilityAnimNotify` in montage fires GameplayEvent with socket transform in TargetData
3. Ability receives event → `FireProjectilesFromSocket(SocketLocation)`

**Montage decoupling**: Abilities don't store montages. Actors map `TriggerTag → UAnimMontage*` in `AbilityMontageMap` property.

**Socket mapping**: `CombatSocket.*` tags → SKM socket names via `USHAbilitySystemLibrary::GetSocketNameForCombatTag()`.

### GameplayTags (key namespaces)

```
DamageType.Physical / .Collision
Faction.Player / .Ally / .Enemy / .Neutral
CombatSocket.Weapon / .LeftHand / .RightHand / .AttackScene
Attribute.Vital.* / .Combat.* / .Block.* / .Player.*
AbilityTrigger.Block.Attack / .Projectile.Linear / .Projectile.Homing / .Melee.Basic / .TimeSlow
ToggleState.Player.TimeSlow
Cooldown.Player.* / .Block.*
SetByCaller.Damage.Multiplier
```

All native tags defined in `FSHGameplayTags` (`SHGameplayTags.h`), initialized in `USHAssetManager::StartInitialLoading()`.

---

## Workflows

### Adding a New Block Type

1. Create `USHPlayerBlockData` (or Enemy/Neutral) DataAsset in Content Browser
2. Fill `LevelConfigs` with `FBlockLevelConfig` per level (stats + `GrantedAbilities`)
3. Set `FSHAIConfig` (detection/attack range)
4. Register DataAsset in `ASHGameStateBase` registry (or use `ASHWorldBlockSpawner` for enemy blocks)
5. Spawn via `ASHPlayerBlock::SpawnDeferred(World, BlockData, Grid, Row, Col, Level, HP, Energy)`

### Adding a New Ability

1. Subclass `USHGameplayAbilityBase` (or existing subclass like `Projectile`)
2. Create a `USHAbilityDataBase` subclass DataAsset
3. Set `AbilityClass` and `TriggerTag` in the DataAsset
4. Add `TriggerTag → UAnimMontage*` entry in the actor's `AbilityMontageMap`
5. Place `USHAbilityAnimNotify` in the montage at fire frame
6. Grant via `FGrantedAbilityConfig` in block's `FBlockLevelConfig` or character init

### Carrying a Block

1. Player calls `ServerCarryBlock()` → `ASHGridBase` removes block from grid
2. Block calls `CreateCarryState()` → `FBlockCarryState {BlockTypeID, Level, HP, Energy}`
3. `ASHPlayerCharacterBase::StartCarrying(FBlockCarryState)` stores state + sets carry meshes (replicated)
4. On place: `ServerPlaceBlock()` → reads carry state → `SpawnDeferred` → `InitializeBlock`

### Level Up / Merge

- `ASHPlayerBlock::UpgradeToLevel(NewLevel)` — refreshes attributes + abilities from new `FBlockLevelConfig`
- `ASHPlayerBlock::MergeWith(OtherBlock)` — caller block absorbs other, caller goes to `Level + 1`

### Time Slow

1. `USHGameplayAbility_TimeSlow::ActivateAbility()` → `GameState->IncrementTimeSlowCount()`
2. `GlobalTimeScale` changes → `OnGlobalTimeScaleChanged` broadcast
3. All registered actors update `CustomTimeDilation`; projectiles scale their `DeltaTime`
4. On end → `GameState->DecrementTimeSlowCount()` (reference counted, safe for multiplayer)

---

## Known Issues / Notes

| Area | Note |
|---|---|
| **MotionWarping** | Plugin is enabled but not yet fully integrated |
| **Faction naming** | `ASHPlayerBlock` uses `Faction.Ally`, not `Faction.Player`. `AreActorsFriends()` treats Player+Ally as the same side |
| **NeutralBlock conversion** | Conversion to `ASHPlayerBlock` on grid overlap — full implementation details may vary by Blueprint subclass |
| **Carry state on death** | If player dies while carrying, carry state handling depends on respawn flow in `ASHGameModeBase` |
| **Grid coord system** | Grid coordinates are **global**, not relative to the grid actor. Use `SHGridFunctionLibrary` helpers — do not compute manually |
| **Ability cost** | Energy cost is checked and consumed in `CheckAndCommitAbilityCost()`. Cooldowns use GameplayTag + timer (not a GE cooldown), so standard GE cooldown queries won't work |
| **Penetration tracking** | `ProcessedCollisions` weak set on projectiles prevents double-hit. Cleared on re-use if projectile is pooled |
| **Chinese strings** | All `FText` display names and in-code comments are in Chinese — expected behavior |
| **FunctionalSKM** | Optional SKM on blocks — must be configured in Blueprint. Static mesh is always present |

---

## Key Architectural Rules

1. **Never partially initialize a block.** Always use `ASHCombatBlockBase::InitializeBlock(FBlockInitParams)`.
2. **Player ASC lives on `ASHPlayerState`**, not the character. Access via `PlayerState->GetAbilitySystemComponent()`.
3. **Faction checks via library helpers only.** Use `USHAbilitySystemLibrary::AreActorsFriends/Enemies()` — never query tags directly.
4. **Time-slow is reference-counted.** Call `IncrementTimeSlowCount` / `DecrementTimeSlowCount` in pairs.
5. **All blocks use `SpawnDeferred` factory methods.** Never call `SpawnActor<>` directly on block classes.
6. **Shared structs `FGridCell` and `FBlockCarryState` are in `Save/SHBlockTypes.h`.** Do not redefine.
7. **Ability DataAssets are passed as `Spec.SourceObject`.** Retrieve with `GetAbilityData<T>()` inside the ability.
