import unreal

MARK = "LOOT15"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = unreal.GameplayStatics.get_player_pawn(world, 0)
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)
pickup_cls = unreal.load_object(None, "/Game/Loot/BP_LootPickup.BP_LootPickup_C")
enemy_cls = unreal.load_object(None, "/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C")

enemies = unreal.GameplayStatics.get_all_actors_of_class(world, enemy_cls)
alive = [e for e in enemies if not e.get_editor_property("Dead")] if enemies else []
unreal.log(MARK + ": enemies=%d alive=%d" % (len(enemies), len(alive)))

# Phase 1: potion restyle check - move player far away first so the drop survives
home = player.get_actor_location()
player.set_actor_location(home + unreal.Vector(0, 0, 5000), False, False)
src = alive[0] if alive else enemies[0]
pP = lib.call_method("SpawnLootDrop", (src, 1.0, 0.0))
mc = pP.get_component_by_class(unreal.StaticMeshComponent)
mesh = mc.get_editor_property("static_mesh") if mc else None
mats = mc.get_editor_property("override_materials") if mc else []
scale = mc.get_editor_property("relative_scale3d") if mc else None
runtime_mat = mc.get_material(0) if mc else None
unreal.log(MARK + ": P1 potion pickup mesh=%s runtime_mat=%s scale=%s" % (
    mesh.get_name() if mesh else "<none>",
    runtime_mat.get_name() if runtime_mat else "<none>", str(scale)))
# consume it to clean up
potions0 = player.get_editor_property("HealthPotionCount ")
player.set_actor_location(pP.get_actor_location(), False, False)
potions1 = player.get_editor_property("HealthPotionCount ")
unreal.log(MARK + ": P1 consume potions %s -> %s" % (potions0, potions1))

# Phase 2: real death chain on a living enemy
if alive:
    victim = alive[0]
    xp0 = player.get_editor_property("CurrentXP ")
    n0 = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
    victim.call_method("Die")
    xp1 = player.get_editor_property("CurrentXP ")
    n1 = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
    unreal.log(MARK + ": P2 Die(%s): XP %s->%s (chain ran if increased), pickups %d->%d" % (
        victim.get_name(), xp0, xp1, n0, n1))
else:
    unreal.log(MARK + ": P2 skipped, no living enemies")

# Phase 3: statistical Die test with freshly spawned enemies (5 samples)
melee_cls = unreal.load_object(None, "/Game/Enemies/BP_Enemy_Melee.BP_Enemy_Melee_C")
spawned_drops = 0
samples = 0
for i in range(5):
    t = unreal.Transform()
    t.translation = home + unreal.Vector(2000 + i * 400, 2000, 200)
    e = unreal.GameplayStatics.begin_deferred_actor_spawn_from_class(
        world, melee_cls, t, unreal.SpawnActorCollisionHandlingMethod.ALWAYS_SPAWN)
    if not e:
        unreal.log(MARK + ": P3 spawn %d failed" % i)
        continue
    unreal.GameplayStatics.finish_spawning_actor(e, t)
    n_before = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
    try:
        e.call_method("Die")
    except Exception as ex:
        unreal.log(MARK + ": P3 Die failed on sample %d: %s" % (i, str(ex)))
        continue
    n_after = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
    samples += 1
    if n_after > n_before:
        spawned_drops += 1
unreal.log(MARK + ": P3 statistical: %d/%d Die() calls dropped loot (expect ~70%%)" % (spawned_drops, samples))

# cleanup: walk over every remaining pickup
remaining = unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls)
unreal.log(MARK + ": cleanup %d pickups" % len(remaining))
for p in remaining:
    player.set_actor_location(p.get_actor_location(), False, False)
player.set_actor_location(home, False, False)
gold = lib.call_method("GetGold", (player,))
potions = player.get_editor_property("HealthPotionCount ")
unreal.log(MARK + ": FINAL gold=%s potions=%s" % (gold, potions))
