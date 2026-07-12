
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
print("LMCHK|LOOT_MENU_INSTANCES", len(menus))
for m in menus:
    try: act = m.is_activated()
    except Exception: act = "n/a"
    print("LMCHK|MENU", m.get_name(), "vis", m.get_visibility(), "activated", act, "inviewport", m.is_in_viewport())
    # inspect the Them panel item count via widget tree name InventoryItemBox in the them inventory
