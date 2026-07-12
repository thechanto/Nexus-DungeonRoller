import unreal
def mark(k,v): unreal.log("S9|%s|%s"%(k,v))
gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc=unreal.GameplayStatics.get_player_controller(gw,0)

# HUD proof: UpdateGoldDisplay reads GetRunInventoryGold and writes GoldText
try:
    g=unreal.NexusAbilityUILibrary.get_run_inventory_gold(gw)
    ok=unreal.NexusAbilityUILibrary.update_gold_display(gw)
    mark("HUD","get_run_inventory_gold=%s update_gold_display_returned=%s"%(g,ok))
except Exception as e:
    mark("HUD_ERR", e)

# EI subsystem
sub=None
try:
    lp=pc.get_editor_property("player")
    mark("PLAYER_CLASS", lp.get_class().get_name() if lp else "None")
    sub=lp.get_subsystem(unreal.EnhancedInputLocalPlayerSubsystem)
except Exception as e:
    mark("GETSUB_ERR", e)
mark("EI_SUBSYS", sub.get_name() if sub else "None")

# menu open?
def find_menu():
    for w in unreal.WidgetLibrary.get_all_widgets_of_class(gw, unreal.CommonActivatableWidget, False):
        if 'Inventory' in w.get_class().get_name(): return w
    return None
menu=find_menu()
if not menu:
    for c in pc.get_components_by_class(unreal.NexusInventoryUIComponent):
        c.toggle_inventory_menu()
    menu=find_menu()
mark("MENU_OPEN", menu.get_name() if menu else "None")

if sub and menu:
    ia=unreal.load_object(None,"/Game/Input/Actions/IA_Inventory.IA_Inventory")
    sub.inject_input_vector_for_action(ia, unreal.Vector(1,0,0), [], [])
    mark("INJECTED","press queued")
else:
    mark("INJECTED","skipped sub=%s menu=%s"%(bool(sub),bool(menu)))
mark("DONE","ok")
