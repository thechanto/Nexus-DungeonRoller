
import unreal
def desc(inv):
    out=[]
    for it in inv.get_items():
        try: q=it.get_editor_property("quantity")
        except Exception: q="?"
        out.append((it.get_class().get_name(), q))
    return out
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pc = unreal.GameplayStatics.get_player_controller(world, 0)
ps = pc.get_editor_property("player_state")
runInv=[c for c in ps.get_components_by_class(unreal.NarrativeInventoryComponent) if "Run" in c.get_name()][0]
chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chest = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)[0]
chestInv = chest.get_component_by_class(unreal.NarrativeInventoryComponent)
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
print("FINAL|MENU_OPENED_COUNT", len(menus))
for m in menus:
    print("FINAL|MENU_STATE", m.get_name(), "activated", m.is_activated())
# take all chest items into run inventory
for it in list(chestInv.get_items()):
    try: q = it.get_editor_property("quantity")
    except Exception: q = 1
    runInv.request_loot_item(it, q)
print("FINAL|RUNINV_AFTER", desc(runInv))
print("FINAL|CHEST_AFTER", desc(chestInv))
# close menu (proves close path)
for m in menus:
    before=m.is_activated(); m.deactivate_widget()
    print("FINAL|MENU_CLOSE", m.get_name(), "was", before, "now", m.is_activated())
