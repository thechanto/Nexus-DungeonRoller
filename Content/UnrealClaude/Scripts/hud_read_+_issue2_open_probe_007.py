import unreal
def mark(k,v): unreal.log("ISSUE2|%s|%s"%(k,v))

gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc=unreal.GameplayStatics.get_player_controller(gw,0)

# --- HUD GoldText read ---
hud=pc.get_editor_property("HudRef")
mark("HUD_REF", hud.get_name() if hud else "None")
if hud:
    try:
        gt=hud.get_widget_from_name("GoldText")
        mark("GOLDTEXT", (gt.get_text() if gt else "None(name lookup)"))
    except Exception as e:
        mark("GOLDTEXT_ERR", e)

# --- component + open menu ---
comp=None
for c in pc.get_components_by_class(unreal.NexusInventoryUIComponent):
    comp=c
mark("COMP", comp.get_name() if comp else "None")
if comp:
    mark("TOGGLE_ACTION", comp.get_editor_property("toggle_action").get_name() if comp.get_editor_property("toggle_action") else "None")
    comp.toggle_inventory_menu()

# find the open menu widget
menu=None
try:
    for w in unreal.WidgetLibrary.get_all_widgets_of_class(gw, unreal.CommonActivatableWidget, False):
        if 'Inventory' in w.get_class().get_name():
            menu=w
except Exception as e:
    mark("MENU_FIND_ERR", e)
if menu:
    mark("MENU_OPEN","%s activated=%s"%(menu.get_name(), menu.is_activated() if hasattr(menu,'is_activated') else menu.get_editor_property('is_activated') if False else 'n/a'))
else:
    mark("MENU_OPEN","none found")

# --- probe EI subsystem reachability for injection ---
try:
    lp=pc.get_local_player()
    mark("LOCAL_PLAYER", lp.get_name() if lp else "None")
    sub=lp.get_subsystem(unreal.EnhancedInputLocalPlayerSubsystem) if lp else None
    mark("EI_SUBSYS", sub.get_name() if sub else "None")
except Exception as e:
    mark("EI_SUBSYS_ERR", e)

mark("DONE","ok")
