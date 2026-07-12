import unreal
def mark(k,v): unreal.log("S8|%s|%s"%(k,v))
gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc=unreal.GameplayStatics.get_player_controller(gw,0)

# HUD GoldText via widget tree
hud=pc.get_editor_property("HudRef")
try:
    wt=hud.get_editor_property("widget_tree")
    found=False
    for w in wt.get_all_widgets():
        n=w.get_name()
        if 'Gold' in n and isinstance(w, unreal.TextBlock):
            mark("GOLDTEXT","%s = '%s'"%(n, w.get_text())); found=True
    if not found: mark("GOLDTEXT","no Gold TextBlock in tree")
except Exception as e:
    mark("WIDGETTREE_ERR", e)

# EI subsystem via 'player' property
lp=pc.get_editor_property("player")
mark("PLAYER", (lp.get_class().get_name(), lp.get_name()) if lp else "None")
sub=None
try:
    sub=lp.get_subsystem(unreal.EnhancedInputLocalPlayerSubsystem)
except Exception as e:
    mark("GETSUB_ERR", e)
mark("EI_SUBSYS", sub.get_name() if sub else "None")

# menu currently open?
menu=None
for w in unreal.WidgetLibrary.get_all_widgets_of_class(gw, unreal.CommonActivatableWidget, False):
    if 'Inventory' in w.get_class().get_name(): menu=w
mark("MENU_NOW", menu.get_name() if menu else "None")

# if reachable + menu open, inject a press of IA_Inventory
if sub and menu:
    ia=unreal.load_object(None,"/Game/Input/Actions/IA_Inventory.IA_Inventory")
    sub.inject_input_vector_for_action(ia, unreal.Vector(1,0,0), [], [])
    mark("INJECTED","IA_Inventory value=1 (close expected next tick)")
else:
    mark("INJECTED","skipped (sub=%s menu=%s)"%(bool(sub),bool(menu)))
mark("DONE","ok")
