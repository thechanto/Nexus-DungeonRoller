import unreal
def L(m): unreal.log("ISSUE1C " + str(m))
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
if not world: L("NO PIE"); raise SystemExit
pc = unreal.GameplayStatics.get_player_controller(world, 0)
uicomp = pc.get_component_by_class(unreal.NexusInventoryUIComponent)
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C")

def find_menus():
    return unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)

menus = find_menus()
L("open menus before = %d" % len(menus))
if len(menus) == 0 and uicomp:
    uicomp.toggle_inventory_menu()
    L("toggled open")
    menus = find_menus()
    L("open menus after toggle = %d" % len(menus))

for m in menus:
    L("menu %s visibility=%s activated=%s" % (m.get_name(), m.get_visibility(), m.is_activated()))
    invw = m.get_editor_property("WBP_InventoryWidget")
    bound_inv = invw.get_editor_property("Inventory") if invw else None
    L("  bound Inventory = %s" % (bound_inv.get_name() if bound_inv else None))
    cached = invw.get_editor_property("CachedItems") if invw else []
    L("  CachedItems count = %d" % len(cached))
    for it in cached:
        try: L("    item %s x%d" % (it.get_class().get_name(), it.get_quantity()))
        except Exception as e: L("    item err %s" % e)
L("DONE")
