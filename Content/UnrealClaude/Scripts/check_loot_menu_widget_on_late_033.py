
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Menus/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
print("MENUCHK|LOOT_MENU_INSTANCES", len(menus))
for m in menus:
    vis = m.get_visibility()
    try: act = m.is_activated()
    except Exception: act = "n/a"
    print("MENUCHK|MENU", m.get_name(), "vis", vis, "activated", act, "inviewport", m.is_in_viewport())
# also list any activatable widgets to confirm HUD stack has it
