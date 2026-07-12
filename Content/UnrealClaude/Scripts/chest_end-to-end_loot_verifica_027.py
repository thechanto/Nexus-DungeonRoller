
import unreal
def log(*a): print("CHESTVERIFY|" + "|".join(str(x) for x in a))
def desc(inv):
    out=[]
    for it in inv.get_items():
        try: q=it.get_editor_property("quantity")
        except Exception: q="?"
        out.append((it.get_class().get_name(), q))
    return out

ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
log("WORLD", world.get_name() if world else None)
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
log("PAWN", pawn.get_name() if pawn else None, "PC", pc.get_name() if pc else None)

chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chests = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)
log("CHESTS_FOUND", len(chests))
# report each chest's BeginPlay-populated loot
for ch in chests:
    ci = ch.get_component_by_class(unreal.NarrativeInventoryComponent)
    log("CHEST_BEGINPLAY_LOOT", ch.get_name(), desc(ci))

chest = chests[0]
chestInv = chest.get_component_by_class(unreal.NarrativeInventoryComponent)

ps = pc.get_editor_property("player_state")
log("PLAYERSTATE", ps.get_name() if ps else None)
comps = ps.get_components_by_class(unreal.NarrativeInventoryComponent)
log("PS_INV_COMPS", [c.get_name() for c in comps])
runInv=None
for c in comps:
    if "Run" in c.get_name(): runInv=c
if runInv is None: runInv=comps[0]
log("RUNINV_NAME", runInv.get_name())
log("RUNINV_BEFORE", desc(runInv))

ok = unreal.NexusAbilityUILibrary.open_container_loot(pawn, chest)
log("OPEN_CONTAINER_LOOT_RET", ok)
ls = runInv.get_editor_property("loot_source")
log("LOOTSOURCE_IS_CHEST", ls == chestInv, ls.get_name() if ls else None)

# menu open check
try:
    menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Menus/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
    menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
    log("LOOT_MENU_INSTANCES", len(menus))
except Exception as e:
    log("MENU_CHECK_ERR", str(e)[:120])

# snapshot chest items then loot each into run inventory
snap = list(chestInv.get_items())
log("CHEST_STACKS_TO_LOOT", len(snap))
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
log("DONE")
