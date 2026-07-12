import unreal

MARK = "LOOT16"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = unreal.GameplayStatics.get_player_pawn(world, 0)
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)
pickup_cls = unreal.load_object(None, "/Game/Loot/BP_LootPickup.BP_LootPickup_C")
enemy_cls = unreal.load_object(None, "/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C")
enemies = unreal.GameplayStatics.get_all_actors_of_class(world, enemy_cls)

home = player.get_actor_location()
player.set_actor_location(home + unreal.Vector(0, 0, 8000), False, False)  # out of the way

def mesh_of(p):
    mc = p.get_component_by_class(unreal.StaticMeshComponent)
    m = mc.get_editor_property("static_mesh") if mc else None
    return m.get_name() if m else "<none>"

# Phase 1: potion restyle (player far away so it survives)
pP = lib.call_method("SpawnLootDrop", (enemies[0], 1.0, 0.0))
mc = pP.get_component_by_class(unreal.StaticMeshComponent)
mat = mc.get_material(0) if mc else None
unreal.log(MARK + ": P1 potion mesh=%s mat=%s scale=%s (expect sphere/hex-pulse/0.35)" % (
    mesh_of(pP), mat.get_name() if mat else "<none>",
    str(mc.get_editor_property("relative_scale3d")) if mc else "-"))

# Phase 2: 10x default-args rolls
golds = potions = misses = 0
for i in range(10):
    p = lib.call_method("SpawnLootDrop", (enemies[0], 0.7, 0.6))
    if not p:
        misses += 1
    elif mesh_of(p) == "SM_Cylinder":
        golds += 1
    else:
        potions += 1
unreal.log(MARK + ": P2 rolls: %d gold / %d potion / %d none (expect ~4/3/3)" % (golds, potions, misses))

# Phase 3: real Die() on both placed enemies (Completed pin now drives the drop)
for e in enemies:
    n0 = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
    e.call_method("Die")
    n1 = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
    unreal.log(MARK + ": P3 Die(%s) -> drop=%s" % (e.get_name(), "YES" if n1 > n0 else "no (30%% roll)"))

# Phase 4: cleanup - walk over everything, verify counters move
gold0 = lib.call_method("GetGold", (player,))
potions0 = player.get_editor_property("HealthPotionCount ")
remaining = unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls)
for p in remaining:
    player.set_actor_location(p.get_actor_location(), False, False)
player.set_actor_location(home, False, False)
gold1 = lib.call_method("GetGold", (player,))
potions1 = player.get_editor_property("HealthPotionCount ")
unreal.log(MARK + ": P4 consumed %d pickups: gold %s->%s potions %s->%s" % (
    len(remaining), gold0, gold1, potions0, potions1))
unreal.log(MARK + ": P4 HUD GoldText present=%s" % str(lib.call_method("UpdateGoldDisplay", (player,))))
