import unreal

MARK = "LOOT08"
HUD_PATH = "/Game/Widgets/W_PlayerHUD"
BACKUP_PATH = "/Game/Widgets/W_PlayerHUD_PreGoldBackup"

# 1) backup copy before touching the widget tree
if not unreal.EditorAssetLibrary.does_asset_exist(BACKUP_PATH):
    dup = unreal.EditorAssetLibrary.duplicate_asset(HUD_PATH, BACKUP_PATH)
    unreal.log(MARK + ": backup duplicate -> " + str(dup))
    unreal.EditorAssetLibrary.save_asset(BACKUP_PATH)
else:
    unreal.log(MARK + ": backup already exists")

wb = unreal.load_asset(HUD_PATH)
tree = wb.get_editor_property("widget_tree")
root = tree.get_editor_property("root_widget")
unreal.log(MARK + ": root widget = %s (%s)" % (root.get_name(), root.get_class().get_name()))

existing = [w for w in tree.get_editor_property("all_widgets")] if False else []
# check for an existing GoldText
found = None
def walk(w):
    global found
    if w is None:
        return
    if w.get_name() == "GoldText":
        found = w
    if isinstance(w, unreal.PanelWidget):
        for i in range(w.get_children_count()):
            walk(w.get_child_at(i))
walk(root)
if found:
    unreal.log(MARK + ": GoldText already exists, skipping create")
else:
    if not isinstance(root, unreal.PanelWidget):
        raise Exception("root is not a panel: " + root.get_class().get_name())
    tb = unreal.new_object(unreal.TextBlock, outer=tree, name="GoldText")
    tb.set_text(unreal.Text("Gold: 0"))
    try:
        tb.set_editor_property("color_and_opacity",
            unreal.SlateColor(unreal.LinearColor(0.788, 0.647, 0.361, 1.0)))
    except Exception as e:
        unreal.log(MARK + ": color set failed: " + str(e))
    try:
        font = tb.get_editor_property("font")
        font.set_editor_property("size", 24)
        tb.set_editor_property("font", font)
    except Exception as e:
        unreal.log(MARK + ": font size set failed: " + str(e))

    slot = root.add_child(tb)
    unreal.log(MARK + ": added to root, slot = " + str(slot))
    if isinstance(slot, unreal.CanvasPanelSlot):
        slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(1.0, 0.0), maximum=unreal.Vector2D(1.0, 0.0)))
        slot.set_alignment(unreal.Vector2D(1.0, 0.0))
        slot.set_position(unreal.Vector2D(-40.0, 40.0))
        slot.set_auto_size(True)
        slot.set_z_order(50)
        unreal.log(MARK + ": canvas slot anchored top-right")
    else:
        unreal.log(MARK + ": slot is %s, left default layout" % str(type(slot)))

unreal.BlueprintEditorLibrary.compile_blueprint(wb)
saved = unreal.EditorAssetLibrary.save_asset(HUD_PATH)
unreal.log(MARK + ": compiled + save -> " + str(saved))
