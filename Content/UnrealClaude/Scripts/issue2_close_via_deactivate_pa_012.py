import unreal
def mark(k,v): unreal.log("S12|%s|%s"%(k,v))
gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc=unreal.GameplayStatics.get_player_controller(gw,0)
def find_menu():
    for w in unreal.WidgetLibrary.get_all_widgets_of_class(gw, unreal.CommonActivatableWidget, False):
        if 'NarrativeMenu_Inventory' in w.get_class().get_name(): return w
    return None
m0=find_menu()
mark("MENU_BEFORE", "%s activated=%s"%(m0.get_name(), m0.is_activated()) if m0 else "None")
comp=None
for c in pc.get_components_by_class(unreal.NexusInventoryUIComponent): comp=c
# ToggleInventoryMenu on an OPEN menu runs the same DeactivateWidget path CloseInventoryMenu uses
comp.toggle_inventory_menu()
m1=find_menu()
mark("MENU_AFTER_TOGGLE", "%s activated=%s"%(m1.get_name(), m1.is_activated()) if m1 else "None(closed)")
mark("CURSOR", pc.get_editor_property("show_mouse_cursor"))
mark("DONE","ok")
