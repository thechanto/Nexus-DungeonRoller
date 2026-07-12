import unreal
def L(m): unreal.log("ISSUE1E " + str(m))
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C")
menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
L("menus=%d" % len(menus))
for m in menus:
    invw = m.get_editor_property("WBP_InventoryWidget")
    box = invw.get_editor_property("InventoryItemBox")
    L("InventoryItemBox=%s childCount=%d" % (box.get_name() if box else None, box.get_children_count() if box else -1))
    if box:
        for i in range(box.get_children_count()):
            ch = box.get_child_at(i)
            item = None
            try: item = ch.get_editor_property("Item")
            except Exception: pass
            L("  child %d: %s item=%s" % (i, ch.get_name(), item.get_class().get_name() if item else '?'))
    # also render/geometry
    L("menu visible=%s render opacity=%s" % (m.is_visible(), m.get_render_opacity()))
L("DONE1E")
