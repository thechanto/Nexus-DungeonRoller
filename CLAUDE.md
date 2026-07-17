# CLAUDE.md — Nexus (Dungeon Roller) Project Brain

You are an autonomous agent working on Nexus, a solo-developed UE 5.7.4 extraction action-RPG. The developer (Chanto) owns game design, PIE testing (feel/visuals/audio), Git commits, and destructive approvals. You own everything else: inspection, C++, Blueprint edits via the UnrealClaude MCP bridge, terminal builds, verification. Read this whole file before acting.

## 1. Project identity

- Project: `C:\Users\chant\OneDrive\Documents\Unreal Projects\Nexus-part-final 5.7\Nexus.uproject`
- Repo: `github.com/thechanto/Nexus-DungeonRoller` (private, Git LFS). GitHub Desktop is the user's commit tool.
- Engine: UE 5.7.4. Plugins: NarrativeInventory + NarrativeEquipment (vendor — treat as read-only rails; the few sanctioned content edits live on a "vendor patch list"), UnrealClaude (the MCP bridge, listener on localhost:3000).
- Maps: `Lvl_MainMenu` (startup), `LV_Soul_Cave` (the dungeon + boss — only cooked gameplay map), `Lvl_ThirdPerson` (editor test level), `/Game/Maps/Dev/Lvl_TestGym` (dev gym — cook-excluded, armory table with all weapon pickups, future loot-spawn pad). MapsToCook = MainMenu + Soul_Cave ONLY. Never add Dev maps to cooking.
- Genre intent: extraction tension is the core. Bring gear = risk gear. Death forfeits the run's unbanked loot/XP; extraction banks it.

## 2. Non-negotiable working rules

1. **Live Coding is BANNED project-wide.** It has masked two real bugs (a stale-DLL "teleport" bug and a unity-build duplicate-symbol error that shipped in a commit). All C++ goes: write → verify from disk → user closes editor → terminal build → reopen. Never suggest CTRL+ALT+F11.
2. **Build command** (editor closed, terminal): `"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" NexusEditor Win64 Development -Project="C:\Users\chant\OneDrive\Documents\Unreal Projects\Nexus-part-final 5.7\Nexus.uproject" -WaitMutex` — PowerShell, not Git Bash (bash mangles the quoted path).
3. **Package command**: `RunUAT.bat BuildCookRun -project=... -platform=Win64 -clientconfig=Development -cook -build -stage -pak -archive -archivedirectory="C:\NexusPackaged"`. Watch for widget GUID ensures (the class of error that kills cooks — W_LockOnIndicator and W_AbilitiesScreen were past offenders).
4. **Commits are the user's.** End every work report with a staging list: exact file paths + a one-line commit message. The user unselects-all in GitHub Desktop, filters by filename, checks your list. Local commits; push at checkpoints. Never run git commit/push yourself unless explicitly told.
5. **Approval protocol**: read-only + announced = proceed. Deletes, system config, plugin-content edits, anything unannounced or destructive = stop and ask. NEVER delete assets proactively.
6. **Backups before mutation**: hash-verified raw file copies to `Content\_Backups\<Task>_<date>\`. Do NOT use duplicate_asset on Blueprints (crashes the editor). Restore = file copy back with editor closed.
7. **Demo values are deliberate, keep them**: DropChance 1.0, P-key pause alias. Chest CDO must stay MinGold=30/MaxGold=80/MinPotions=1/MaxPotions=2 — verify at the end of any session that touched loot/chests.
8. **Verification is readback, not task status.** The bridge false-reports success/cancelled. Ground truth = LogPython / the editor log / disk bytes / CDO readback after compile AND after save.

## 3. C++ / Blueprint conventions

- Include paths use the module prefix for cross-folder includes: `#include "Nexus/Inventory/NexusWeaponItem.h"` (Source/ is the include root).
- CDO edits: **set → compile → save**, in that order. Set-after-compile silently reverts. This has bitten twice.
- Widget-tree edits go through the Blueprint's own tree object (`W_X.W_X:WidgetTree`), never the generated class (`_C:WidgetTree`) — the latter silently reverts on compile.
- Child-BP SCS component overrides: write via the direct `<BP>_C:<Component>_GEN_VARIABLE` sub-path. SubobjectDataSubsystem's gather on a child returns the PARENT's templates — writing through it edits the base class for every child.
- This project has shipped **variable names with trailing spaces** (`ManaPotionCount `, `HealthPotionCount ` on BP_NexusPlayer). Always probe both spellings; use TrimStartAndEnd matching in C++ name lookups.
- Library functions called from GA graphs: no `DefaultToSelf` (self isn't an Actor there); callers wire GetAvatarActorFromActorInfo.
- Log categories: one `DEFINE_LOG_CATEGORY` per category in exactly one .cpp + `DECLARE_LOG_CATEGORY_EXTERN` in a header. `DEFINE_LOG_CATEGORY_STATIC` in two files = unity-build C2011 (this shipped broken once; LogStashView is the precedent — defined in NexusStashView.cpp, declared in NexusStashView.h).
- New widget classes: factory-created widget BPs have NO root widget and root_widget is unscriptable — build runtime trees in C++ `RebuildWidget()`; the BP carries styling as native UPROPERTYs (recompile-proof).
- Calling BP-class functions from C++: reflection (`FindFunction` + `ProcessEvent`), parameters set by iterating parm properties by type, not name.

## 4. Systems map (what exists and how it works)

### GAS / attributes

- `UBasicAttributeSet`: Health/Max, Stamina/Max, Mana/Max, Shield/Max, Damage (meta), Strength, Dexterity, Intelligence, Faith, Level, StatPoints. `UCombatAttributeSet` (Armor, Strength) appears vestigial. UNKNOWN: whether Str/Int/Dex actually feed damage calcs or are display-only — check before building anything on them.
- Ability grants on PossessedBy: `GrantAbilities(StartingAbilities)` stamps InputID from CDO (=None for most) then `GrantAssignedAbilities` stamps slots 5-8 from the save. A **fragile 0.6s delay** sits in this chain (known tech debt). StartingAbilities = EquipWeapon, HitReaction, Death, 7× UpgradeStat — never the class abilities.
- Class abilities (LeapSlash/ShieldSlam/SkyCrusher/SwordSweep/ArcaneBolt/Burden/CosmicRift/Meteor) are `GA_NexusWeaponAbility` children gated by `RequiresEquippedWeapon` (class = weapon category). The ability bar (W_AbilitiesBar) rebuilds on `WeaponsManager.OnWeaponChanged` and filters on that gate.
- HUD: DT_AbilityMetaData keys `GA_*_C` → icon textures (`T_AbilityIcon_*_HUD` alpha variants for the bar; originals stay on the abilities screen). Digit badges read runtime spec InputID via C++ `GetGrantedAbilityInputID`. Mouse wheel (MouseScrollUp/Down → IA_EquipWeaponSlot1/2) is the ONLY weapon-swap input; keys 1-4 are ability binds.

### Weapons & equippables (the mesh-from-item architecture)

- `WeaponsManagerComponent` (BP, on BP_NexusPlayer): GiveWeapon (spawn + stow), EquipWeapon (montage, anim-class swap, socket attach at Equip notify, OnWeaponChanged), UnequipWeapon. StartingWeapons is EMPTY — weapons arrive only via items.
- `UNexusWeaponItem : UEquippableItem` (C++): WeaponClass (category actor), InHandScale, GripOffset, GripRotation, TraceLengthOverride (0 = auto-derive: scaledBoundsTop + GripOffset.Z + 30cm overshoot convention), GroundScale, EquipStatsEffect (exists, unset — phase 2). `ApplyVariantVisuals()` in HandleEquip sets the weapon actor's mesh from the ITEM's PickupMesh — one field drives ground look AND hand look ("what you pick up is what you wield" — user's core principle).
- Toggle guards (both live-verified): Guard 1 — HandleEquip skips EquipWeapon if that class is already in hand (WM's re-request toggle would disarm). Guard 2 — HandleUnequip calls UnequipWeapon only if our class is in hand.
- Model: **weapon class = category** (axe/staff: anims, abilities, sockets), **item = variant** (mesh + tuning + future stats). New variant of an existing category = item BP + data fields + gym eyeball (~minutes). New anim archetype = new BP_Weapon_* + config + tag + GA switch entry.
- Variants: RustedAxe (SM_Axe 1.10, +26Z), ApprenticeStaff (SM_Spear 0.60, −31Z), SunderingGreataxe (SM_GreatAxe 0.85, +32Z, **GripRotation Yaw=160**). GripRotation is per-mesh (each has its own authored forward axis) — eyeball in the gym, never assume a constant. Greataxe is NOT in loot tables yet (loot-roll refactor pending).
- **NEVER edit skeleton sockets**: `staff_equipped_socket` doubles as GA_AOEAttack's targeting origin. All fit adjustments go on WeaponMesh relative transform / item tuning fields. Stowed sockets are cosmetic-only exception.
- Melee trace runs TraceStart→TraceEnd components (actor-relative, mesh-blind); the axe's tuned TraceEnd = 122.4 (the +30cm overshoot convention preserved from the original author).

### Inventory / stash / loadout

- Two `UNarrativeInventoryComponent`s on **BP_NexusPlayerState**: RunInventory (12 slots/40 weight, NO load path — starts empty every run) and Stash (50/10000, `Load("NexusStash")` on PS BeginPlay).
- PS BeginPlay chain: `LoadStash → SeedStartingWeaponItems` (grants axe+staff items per run; dedupe = HasItem on Run AND Stash with bCheckVisibility=false, so equipped/hidden copies count — no free-weapon pileup).
- Equipped items leave the bag list (`ShouldShowInInventory` override: `!bActive && Super`). `SetActive` broadcasts nothing — call `ClientRefreshInventory()` after equip/unequip.
- Extraction (`ExtractRunInventoryToStash`): deactivate-all guard first (weapon lowered, no bActive round-trips into the stash save), then move-all to Stash, save. Death: `ClearRunInventory` (loss is the design).
- Main-menu stash: BP_MainMenuGameMode has `PlayerStateClass = BP_NexusPlayerState` — the menu runs the REAL inventory stack (stash loads, seeder runs there too; accepted). `UNexusMainMenu` (Button_Stash) opens `UNexusStashView` (C++ runtime tree hosting plugin `WBP_Loot_TheirInventory.InitializeFromInventory(Stash)`). Gold readout reads `Stash->GetCurrency()`, renders bottom-left — user likes it there, keep it.
- Planned next (specs agreed): three-zone loadout screen (stash pane + RunInventory basket + reused inventory-screen player preview), MOVED semantics, `"NexusLoadout"` save-slot handoff at Play → `LoadRunLoadout()` spliced between LoadStash and Seed (delete slot after load — crash-recovery friendly). Marketplace rides on top later: third pane + gold-spend; everything buyable for testing, prices as data knobs, genre-balance deferred.

### Loot

- `ENexusLootType`: Gold, HealthPotion, ManaPotion, WeaponRustedAxe=3, WeaponApprenticeStaff=4. Item class paths in `GLootFields` (single source of truth — the seeder uses the same paths). Weapon share: GWeaponShare=0.08 nested pre-roll, 50/50 axe/staff, enemy kills only.
- Hybrid pickup grammar (deliberate): gold/potions = `BP_LootPickup` spinning walk-over tokens; weapons = `BP_DroppedItemPickup` static E-interact with named prompt (`RefreshDroppedItemPrompt` on BeginPlay; ItemClass/QuantityToGive are instance-editable — the gym uses this).
- `SpawnBossLoot(DeadBoss, 150, 100)`: guaranteed 5-item ring (72° + random offset, per-point floor traces) spliced into BP_Enemy_Boss On Death after RemoveBossHealthBar; the inherited random roll stays as a bonus 6th.
- `DropScaleForItem` reads GroundScale off the item CDO (no hardcodes).
- Potion flasks: HUD counts are CDO vars on BP_NexusPlayer (`ManaPotionCount `=1, `HealthPotionCount `=2, trailing spaces). Potion PICKUPS are separate NarrativeItems (BP_Item_HealthPotion/ManaPotion) — using one fires GrantPotion (flask +1) and consumes the item.

### Death / XP / attack tokens (BUGS IN FLIGHT — see §6)

- Attack-token system (Elzoheiry pattern): enemies reserve tokens from targets (`ReservedAttackTokens` map on BP_Enemy_Base) before attacking, return them after.
- Enemy death chain (BP_Enemy_Base, traced with GUIDs in §6): head → SetCollisionEnabled(?) → StopLogic(brain) → SetStateAsDead(AIC) → hide HealthBar → ForEach(ReservedAttackTokens.Keys){ReturnAttackToken → Cast(GetPlayerPawn) → AddXP} → Completed → SpawnLootDrop.
- Player death: GA_Death applies GE_Death, ClearRunInventory, 2s Delay → death screen → input mode UI-only.

### Settings / audio

- `UNexusGameUserSettings`: MasterVolume (floored, config), MouseSensitivity (config; BP_NexusPlayer's look-multiplies read static `GetNexusMouseSensitivity()`). `UNexusGameInstance` applies volume at Init AND OnStart to ALL audio devices (PIE uses a separate device — single-device writes silently fail). Window-mode PIE-degrades to WindowedFullscreen. No SoundClass assets exist; SetTransientPrimaryVolume is shared with camera-fade (documented tradeoff).

## 5. Tooling landmines (bridge/Python/MCP)

- Function graphs are UNREACHABLE by the MCP blueprint tools and by Python (only EventGraph). If a fix needs function-graph wiring, the USER hand-wires it from your node-by-node walkthrough; verify afterward via T3D export.
- `delete_asset` and `reload_packages` pop blocking modal dialogs that freeze the bridge or crash the editor. Avoid; if unavoidable, warn the user first.
- Python access-violations (python311 → PythonScriptPlugin → CoreUObject) can crash the editor mid-operation — but past crashes reconciled clean (work landed before dying). After ANY crash: reconcile from disk before re-running anything; check git status for scope, verify assets open, compare against backups. Never blind-rerun mutations.
- `reparent_blueprint` is heavy — run it isolated (fresh asset load right before, nothing chained after in the same script).
- Fab pack mesh first-load stalls the game thread 8-10 min (one-time DDC build) — looks hung, isn't. Arm a log watcher, don't kill it.
- execute_script HTTP route: `description` param required; PowerShell `Get-Content -Raw` produces a PSObject that serializes wrong (use `[IO.File]::ReadAllText` or UTF-8 byte body); `task_status` can false-report "cancelled" while the script completes — LogPython is truth; `json indent=1` breaks log chunking.
- Widget GUID ensures: "did not get a GUID" self-heals in-compile; "deleted but still has GUID" needs compile→save→compile until silent (same ensure class that kills cooks).
- Test data hygiene: verify-scripts that bank items MUST use a throwaway save slot or a byte-exact .bak round-trip (copy → test → restore → hash-verify). A test-seed into the real NexusStash.sav once caused a false "weapon swap is dead" bug (seed-dedupe veto) and a wasted detour.
- SaveGames: `NexusStash.sav` (stash items + currency), controller save (stat points etc.), planned `NexusLoadout`. InventorySaveGame is plain LoadGameFromSlot-readable.
- Live-widget inspection: `unreal.WidgetLibrary` (not WidgetBlueprintLibrary) for live trees; readback properties AND pixels-affecting state (a "verified" white-on-white digit was invisible for a whole session).
- MCP blueprint tools only see BP-declared variables/functions — they CANNOT reference INHERITED NATIVE components or props. `VariableGet CapsuleComponent` → "Variable not found"; `CallFunction GetCapsuleComponent` → "Function not found" (inline getter, no UFUNCTION). So enemy "collision-off on death" couldn't be built via the bridge — user hand-wires it (drag Capsule Component from the Components panel → SetCollisionEnabled NoCollision) or it goes in C++. (Same reason GetRootComponent etc. aren't addable.)
- Bash tool on this machine: call `py`, NOT `python` (the `python` alias errors "Python was not found / Microsoft Store"). Bites when building execute_script JSON payloads or parsing task_result with an inline interpreter.
- Editor-asset widget trees via Python are fenced off: `WidgetBlueprint.get_editor_property('WidgetTree')` is PROTECTED; on the load_object'd `…:WidgetTree`, `get_editor_property('root_widget')` → "property not found" and `get_all_widgets()` isn't a Python method; and `unreal.log()` didn't surface in execute_script's captured `output` (use `print()` → stdout). Net: the gold-HUD-removal was bridge-blocked — needs the correct WidgetTree API or a manual UMG edit.

## 6. Current bug/task state (as of 2026-07-18)

### In flight (open — one work order)

1. **Ability-lifecycle: damage-after-death (paths b+c) + stuck enemies** (BP_Enemy_Base + enemy GAS). Damage-after-death is MULTI-PATH. (a) Weapon hit-scan timer — **FIXED**: death chain calls `WeaponActor→HitScanEnd`, guarded by `IsValid(WeaponActor)` (the earlier `IsWieldingWeapon` guard had NO setter in the base → the fix was silently skipped; guard corrected 2026-07-18). (b) Enemy **GameplayAbilities aren't cancelled on death** — OPEN. (c) `GA_AOEAttack` spawns a persistent `BP_AOE_OverTime_Base` field with its OWN looping damage timer + Duration that outlives the caster (the "rotational AOE after death") — OPEN. Fix: a C++ BlueprintCallable `CancelAllActiveAbilities()` on ANexusEnemyBase (CancelAllAbilities is NOT BP-exposed) called in the death chain, + an "instigator dead → end field" check in shared **BP_AOE_Base** — CAUTION: player abilities (Meteor/CosmicRift) share that base, gate + test. The SAME gap likely drives **stuck enemies** (warriors walk in place, mage freezes, phase-2 "only teleports, never attacks" = attack ability never ends → token never returned → can't re-attack); also check navmesh/terrain (walk-in-place) + attack-token return.
2. **Weapon/class-dependent keybinds — design task**. Ability keybind slots are one GLOBAL set: assigning slot 1 to a mage skill removes it from the warrior, and holding a staff fires the mage skill on key 1 regardless of binding. Want per-weapon-category keybind loadouts (axe→1 = warrior skill, staff→1 = mage skill) that swap on weapon change and persist per category. The ability BAR already filters by RequiresEquippedWeapon (§4); it's the slot ASSIGNMENT + save that's global. Needs a design pass on the save model + equip-time swap.

### Specced, awaiting build

3. **Progression system** (plumbing now KNOWN — inspected 2026-07-17). XP is a TRANSIENT BP var on BP_NexusPlayer (BP `AddXP(double)`, no C++, never saved). Level + StatPoints are GAS attributes (UBasicAttributeSet): Level clamp 1–10 (default 1), StatPoints clamp 0–99 (C++ default 10, gameplay value seeded from save; fresh = 100). NO level-up logic exists anywhere (PreAttributeChange only clamps; PostAttributeChange only refills Max* + fires GameplayAbility.Death at Health≤0). StatPoints are spent by UNexusAbility_UpgradeStat. Save (BP_SaveGame → S_PlayerTalentData) persists StatPoints + TalentPoints ONLY — not Level/XP → nothing banks XP on extraction, nothing forfeits on death. BUILD: add persisted XP (+ cached Level) to the save; add level-up (threshold table, e.g. XPtoNext = 100 × 1.5^(level−1), grant +StatPoints on level-up — target "10 StatPoints" fresh, change from demo 100); bind Level + an "80/100" XP readout on the Skills & Abilities screen; bank XP on extraction, forfeit on death. Do the level-up grant in C++ (authoritative).
4. **Extraction point in LV_Soul_Cave** (root cause found 2026-07-17 — NOT a missing-logic bug). The WORKING extraction actor is **BP_InteractionTest** (`/Game/Test/`, IS placed in LV_Soul_Cave): implements BPI Interactable → Event Interact → Cast player → Save All Data → ExtractRunInventoryToStash → Open Level (Lvl_MainMenu), with Event ShowInteractPrompt → W_InteractPrompt proximity prompt (hidden on BeginPlay). **BP_ExtractPoint** (`/Game/Maps/`) is a DEAD STUB and unplaced (Sphere overlap → Cast → dead-end; no interface/prompt/extraction call; 0 referencers). So "doesn't work" is placement/presentation, not logic. BUILD: promote BP_InteractionTest out of /Game/Test with a real mesh (or port its graph onto BP_ExtractPoint), place ONE at the intended spot, retire the other; PIE-verify the [E] prompt shows and interacting returns to menu.

### Backlog (ordered roughly by pull)

- Loot-roll refactor: weighted array of item paths so variants (greataxe) can DROP; each future variant = one data line.
- Phase-2 stats-on-equip: 2-3 GEs (axe +Str, staff +Int/+MaxMana, greataxe +Str) via the waiting EquipStatsEffect field. FIRST check whether attributes feed damage or are display-only — that decides if this phase is real or cosmetic.
- Per-variant thumbnails (icon bake pipeline exists from the ability-icon work).
- Mana regen scaling (flat 0.4/s now); gold HUD styling; weapon-wheel radial (far-future); per-instance weapon variety (item carries mesh → already true; per-instance INSTANCES would need instance data).
- Fragile 0.6s GrantAssignedAbilities delay; hotkey↔item active-state drift (parked); GA_EquipWeapon Default→UnequipWeapon branch bypasses items (unreachable from shipping inputs).
- Hygiene: dead GetAbilityIconForAbility C++; unreferenced old SM_Axe/SM_Staff meshes; KeyIcon index-0 warning per warrior hex (optional third-Select hardening); 28 cook warnings (montage sync, pose assets, duplicate UpperBody slot node); stray 4.3GB kernel dump; LabelFont 48 on other cloned widgets; GPU probation formal close ~2026-07-21 (day-1 clean, driver 610.74, GPU_PROBATION.md gitignored).
- QoL from 2026-07-18 PIE: remove redundant gold from W_PlayerHUD top-right (also in inventory) — bridge-blocked, see §5 widget-tree, do via correct WidgetTree API or a manual UMG delete (a W_PlayerHUD_PreGoldBackup exists); enemy collision-off on death (walk through corpses) — bridge-blocked, see §5 inherited-component, hand-wire capsule or C++; enemies dissolve/disappear on death ("Thanos snap", VFX/design — a plain destroy-after-delay would also kill the weapon-timer path + collision, but NOT the separate AOE field).

## 7. Decisions and WHY (do not silently reverse these)

- **Ride Narrative plugin rails, never parallel systems** — every equip/inventory feature uses its hooks (ShouldShowInInventory, HandleEquip, EquipmentComponent slots). Zero plugin C++ edits; content edits only via the vendor patch list.
- **Items are trigger+persistence; WeaponsManager is the sole execution layer** — all equip paths converge on it.
- **MOVED (not copied) loadout semantics** — brought items are deducted from the stash save at Play; death loses them. Extraction tension is the game. Same rule extends to XP banking.
- **Hybrid loot grammar** — instant walk-over for currency/consumables, deliberate E-interact for equipment (contents visible before commitment).
- **Mesh-from-item** — PickupMesh drives ground AND hand ("what you see is what you wield"); class=category, item=variant, so new variants are data entry.
- **Extract-deactivate + seeder-dedupe guards** — no auto-re-equip from stash loads, no free-weapon accumulation across extract loops.
- **Save-slot handoff (NexusLoadout) over GameInstance state** — survives crashes; plugin serialization is free; menu basket = same 12/40 component config so capacity can't be exceeded by construction.
- **Everything-buyable marketplace is a TESTING config** — mechanism now, balance later via stock/price data knobs. Loadout screen builds first (marketplace shares ~80% of it).
- **Gym level for testing** — every new feature gets a test room; pickups placed by hand beat RNG hunting. Cook-excluded forever.
- **User owns commits** — advisor/agent reports end with staging lists; this caught missed files once already.
- **1 mana / 2 health starting flasks** — deliberate current tuning (aggressive for testing); known retune knob.

## 8. The user's PIE checklist patterns (hand these back after work)

Give the user a short, concrete checklist after each change-set. Standard passes:

- Fresh gameplay PIE: spawn unarmed, 2 weapon items in bag, equip → draw montage + bar flip + digit badges; wheel-swap coherence (bag rows + equipment slot track the hand); ground pickup beside equipped weapon = same mesh.
- Gym (`Lvl_TestGym`): walk the armory table, E each pickup (named prompts), equip each, judge grip/scale/rotation. New variants get tuned here by eyeball ("direction + rough amount" nudges).
- Menu: Stash button → gold bottom-left, title clear, buttons bronze 24pt, Back fits; Back → Play/Settings/SkillTree regression.
- Extract loop: bank loot → next run seeds fresh → stash shows the bank at menu.
- Combat (for the current bug fixes): kill an enemy mid-swing (no posthumous hit), snipe a never-engaged enemy (XP lands), die (input dead immediately, death screen at 2s).
- After any package: smoke the exe from C:\NexusPackaged — menu → stash → full gameplay loop → extract.

## 9. Report format

End every work session with: what changed (files + mechanism), verification evidence (readback values, log lines, zero-ensure confirmation, chest CDO 30/80/1/2), anything discovered-but-not-fixed, the PIE checklist for the user, and the staging list (files + one-line commit message). Flag anything that needs a terminal build or a user decision explicitly. If the bridge dies mid-work: reconcile from disk before resuming; report what landed vs. what didn't.

## 10. Brain maintenance

At the end of every work session, review what this session changed or taught us: new decisions + their WHY, new landmines discovered, bug-state changes (fixed / new / re-triaged), convention changes. Propose concrete edits to this CLAUDE.md (section + exact new text), apply them after my ok, and include "brain updated: yes/no + what" in the final report. Never silently rewrite §7 decisions — flag conflicts instead.

Size discipline: this file has a budget of ~400 lines. When an update would push it past that, condense in the same pass: merge overlapping rules, tighten wording, and move history out — resolved bugs go to docs/BUGLOG.md (one line each: what, root cause, fix, date), completed specs and detailed walkthroughs go to docs/ARCHIVE.md. This file keeps only what a fresh session needs to act safely TODAY: identity, rules, live systems map, open bugs, decisions+why. Never condense away a §7 decision or a §5 landmine — those earn their lines forever. When condensing, show me a before/after diff summary in the report.
