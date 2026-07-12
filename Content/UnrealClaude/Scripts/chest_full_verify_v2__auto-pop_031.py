
import unreal
def log(*a): print("CV2|" + "|".join(str(x) for x in a))
def desc(inv):
    out=[]
    for it in inv.get_items():
        try: q=it.get_editor_property("quantity")
        except Exception: q="?"
        out.append((it.get_class().get_name(), q))
    return out

ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
log("WORLD", world.get_name(), "PAWN", pawn.get_name())

chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chests = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)
for ch in chests:
    ci = ch.get_component_by_class(unreal.NarrativeInventoryComponent)
    log("CHEST_LOOT", ch.get_name(), desc(ci))

chest = chests[0]
chestInv = chest.get_component_by_class(unreal.NarrativeInventoryComponent)
ps = pc.get_editor_property("player_state")
runInv=None
for c in ps.get_components_by_class(unreal.NarrativeInventoryComponent):
    if "Run" in c.get_name(): runInv=c
log("RUNINV_BEFORE", desc(runInv))

# OpenContainerLoot via reflection (stale python wrapper)
lib_cls = unreal.load_class(None, "/Script/Nexus.NexusAbilityUILibrary")
lib_cdo = unreal.get_default_object(lib_cls)
try:
    ret = lib_cdo.call_method("OpenContainerLoot", (pawn, chest))
    log("OPEN_CONTAINER_LOOT_RET", ret)
except Exception as e:
    log("OPEN_ERR", str(e)[:160])
ls = runInv.get_editor_property("loot_source")
log("LOOTSOURCE_IS_CHEST", ls == chestInv)

# loot menu open?
try:
    menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Menus/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
    menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
    log("LOOT_MENU_INSTANCES", len(menus))
except Exception as e:
    log("MENU_ERR", str(e)[:140])

# take each chest stack into RunInventory
snap = list(chestInv.get_items())
log("STACKS_TO_LOOT", len(snap))
for it in snap:
    try: q = it.get_editor_property("quantity")
    except Exception: q = 1
    cn = it.get_class().get_name()
    try:
        res = runInv.request_loot_item(it, q)
        log("LOOT", cn, q, "->", res)
    except Exception as e:
        log("LOOT_ERR", cn, str(e)[:140])

log("RUNINV_AFTER", desc(runInv))
log("CHEST_INV_AFTER", desc(chestInv))

# close the loot menu (proves close path)
try:
    menus2 = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
    for m in menus2:
        try:
            act_before = m.is_activated()
        except Exception:
            act_before = "?"
        try:
            m.deactivate_widget()
            log("MENU_CLOSE", m.get_name(), "was", act_before, "now", m.is_activated())
        except Exception as e:
            log("CLOSE_ERR", str(e)[:120])
except Exception as e:
    log("CLOSE_OUTER_ERR", str(e)[:120])
log("DONE")
