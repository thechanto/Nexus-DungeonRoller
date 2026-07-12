import unreal
def mark(k,v): unreal.log("S11|%s|%s"%(k,v))
gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc=unreal.GameplayStatics.get_player_controller(gw,0)

def find_menu():
    m=None
    for w in unreal.WidgetLibrary.get_all_widgets_of_class(gw, unreal.CommonActivatableWidget, False):
        cn=w.get_class().get_name()
        if 'NarrativeMenu_Inventory' in cn: return w
        if 'Inventory' in cn: m=w
    return m
menu=find_menu()
if not menu:
    for c in pc.get_components_by_class(unreal.NexusInventoryUIComponent): c.toggle_inventory_menu()
    menu=find_menu()
mark("MENU_BEFORE", "%s activated=%s"%(menu.get_name(), menu.is_activated()) if menu else "None")

sub=None
try:
    lp=menu.get_owning_local_player() if menu else None
    mark("LP", lp.get_name() if lp else "None")
    if lp: sub=lp.get_subsystem(unreal.EnhancedInputLocalPlayerSubsystem)
except Exception as e:
    mark("SUB_ERR", e)
mark("EI_SUBSYS", sub.get_name() if sub else "None")

if sub and menu:
    ia=unreal.load_object(None,"/Game/Input/Actions/IA_Inventory.IA_Inventory")
    sub.inject_input_vector_for_action(ia, unreal.Vector(1,0,0), [], [])
    mark("INJECTED","IA_Inventory press queued")
else:
    mark("INJECTED","skipped")
mark("DONE","ok")
