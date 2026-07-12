import unreal
def L(m): unreal.log("ISSUE1B " + str(m))
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
if not world: L("NO PIE"); raise SystemExit

# discover the widget library name
libnames = [n for n in dir(unreal) if 'Widget' in n and ('Library' in n or 'Statics' in n)]
L("widget libs: %s" % libnames)

pc = unreal.GameplayStatics.get_player_controller(world, 0)
uicomp = pc.get_component_by_class(unreal.NexusInventoryUIComponent)
hud = uicomp.get_hud_widget() if uicomp else None
L("hud wrapper = %s" % (hud.get_name() if hud else None))

menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C")
menus = []
try:
    menus = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, menu_cls, False)
except Exception as e:
    L("WBL failed: %s" % e)
    # fallback via EditorUtility? try get_all_widgets on UWidgetBlueprintLibrary alt
    try:
        menus = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, menu_cls, False)
    except Exception as e2:
        L("fallback failed: %s" % e2)
L("open menu instances: %d" % len(menus))
for m in menus:
    vis = m.get_visibility()
    L("menu %s visibility=%s isActivated=%s" % (m.get_name(), vis, m.is_activated() if hasattr(m,'is_activated') else '?'))
    invw = m.get_editor_property("WBP_InventoryWidget")
    bound_inv = invw.get_editor_property("Inventory") if invw else None
    L("  bound Inventory = %s" % (bound_inv.get_name() if bound_inv else None))
    cached = invw.get_editor_property("CachedItems") if invw else []
    L("  CachedItems count = %d" % len(cached))
    for it in cached:
        L("    item %s x%d" % (it.get_class().get_name(), it.get_quantity()))
L("DONE")
