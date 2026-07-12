import unreal

MARK = "LOOT13"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if not world:
    raise Exception("no PIE world")

player = unreal.GameplayStatics.get_player_pawn(world, 0)
unreal.log(MARK + ": player = " + str(player))
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)

gold0 = lib.call_method("GetGold", (player,))
potions0 = player.get_editor_property("HealthPotionCount")
unreal.log(MARK + ": BEFORE gold=%s potions=%s" % (gold0, potions0))

enemy_cls = unreal.load_object(None, "/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C")
enemies = unreal.GameplayStatics.get_all_actors_of_class(world, enemy_cls)
unreal.log(MARK + ": enemies in level = %d" % len(enemies))
if len(enemies) == 0:
    raise Exception("no enemies to test with")

def mesh_name(pickup):
    mc = pickup.get_component_by_class(unreal.StaticMeshComponent)
    m = mc.get_editor_property("static_mesh") if mc else None
    return m.get_name() if m else "<none>"

# --- Test A: guaranteed GOLD drop, walk over it ---
pA = lib.call_method("SpawnLootDrop", (enemies[0], 1.0, 1.0))
unreal.log(MARK + ": A spawned=%s mesh=%s" % (str(pA), mesh_name(pA) if pA else "-"))
player.set_actor_location(pA.get_actor_location(), False, False)
goldA = lib.call_method("GetGold", (player,))
aliveA = unreal.SystemLibrary.is_valid(pA)
unreal.log(MARK + ": A after-walk gold=%s (expect +10) pickup_still_valid=%s (expect False)" % (goldA, aliveA))

# --- Test B: guaranteed POTION drop ---
pB = lib.call_method("SpawnLootDrop", (enemies[0], 1.0, 0.0))
unreal.log(MARK + ": B spawned=%s mesh=%s (expect sphere)" % (str(pB), mesh_name(pB) if pB else "-"))
player.set_actor_location(pB.get_actor_location(), False, False)
potionsB = player.get_editor_property("HealthPotionCount")
aliveB = unreal.SystemLibrary.is_valid(pB)
unreal.log(MARK + ": B after-walk potions=%s (expect +1) pickup_still_valid=%s" % (potionsB, aliveB))

# --- Test C: real death path via the Die event (70% roll, may or may not drop) ---
pickup_cls = unreal.load_object(None, "/Game/Loot/BP_LootPickup.BP_LootPickup_C")
before_c = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
victim = enemies[min(1, len(enemies) - 1)]
victim.call_method("Die")
after_c = len(unreal.GameplayStatics.get_all_actors_of_class(world, pickup_cls))
unreal.log(MARK + ": C Die() on %s -> pickups %d -> %d (70%% roll)" % (victim.get_name(), before_c, after_c))

# --- Test D: HUD GoldText ---
found = lib.call_method("UpdateGoldDisplay", (player,))
unreal.log(MARK + ": D UpdateGoldDisplay found GoldText = %s" % str(found))
try:
    hud_cls = unreal.load_object(None, "/Game/Widgets/W_PlayerHUD.W_PlayerHUD_C")
    huds = unreal.WidgetLibrary.get_all_widgets_of_class(world, hud_cls, True)
    if huds:
        tb = huds[0].call_method("GetWidgetFromName", ("GoldText",))
        if tb:
            unreal.log(MARK + ": D GoldText reads '%s'" % str(tb.call_method("GetText")))
except Exception as e:
    unreal.log(MARK + ": D text readback failed (non-fatal): " + str(e))

goldF = lib.call_method("GetGold", (player,))
potionsF = player.get_editor_property("HealthPotionCount")
unreal.log(MARK + ": FINAL gold=%s potions=%s (started %s / %s)" % (goldF, potionsF, gold0, potions0))
