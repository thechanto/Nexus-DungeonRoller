import unreal
def L(m): unreal.log("ISSUE1D " + str(m))
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pc = unreal.GameplayStatics.get_player_controller(world, 0)
ps = pc.get_editor_property("player_state")
run_inv = None
for c in ps.get_components_by_class(unreal.NarrativeInventoryComponent):
    if c.get_name()=="RunInventory": run_inv=c
items = run_inv.get_items()
L("RunInventory get_items = %d" % len(items))
for it in items:
    L("  item %s x%d" % (it.get_class().get_name(), it.get_quantity()))

menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C")
menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
L("menus=%d" % len(menus))
for m in menus:
    invw = m.get_editor_property("WBP_InventoryWidget")
    L("invw=%s inventory=%s" % (invw.get_name(), invw.get_editor_property("Inventory").get_name()))
    # list functions
    fns = [n for n in dir(invw) if 'refresh' in n.lower() or 'update' in n.lower() or 'initial' in n.lower() or 'filter' in n.lower()]
    L("invw fns: %s" % fns)
    L("CachedItems before = %d" % len(invw.get_editor_property("CachedItems")))
    try:
        cf = invw.get_editor_property("CurrentFilter"); L("CurrentFilter=%s" % (cf.get_name() if cf else None))
        fc = invw.get_editor_property("FilterClass"); L("FilterClass=%s" % (fc.get_name() if fc else None))
    except Exception as e: L("filter read err %s" % e)
    # try manual refresh
    try:
        invw.call_method("Refresh Inventory")
        L("called Refresh Inventory; CachedItems now = %d" % len(invw.get_editor_property("CachedItems")))
    except Exception as e: L("refresh err %s" % e)
    try:
        invw.call_method("Update Inventory Information")
        L("called Update Inventory Information; CachedItems now = %d" % len(invw.get_editor_property("CachedItems")))
    except Exception as e: L("update err %s" % e)
    # re-initialize
    try:
        invw.call_method("Initialize From Inventory", run_inv, None)
        L("re-Initialize; CachedItems now = %d" % len(invw.get_editor_property("CachedItems")))
    except Exception as e: L("init err %s" % e)
L("DONE")
