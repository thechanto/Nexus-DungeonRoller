import unreal

MARK = "LOOT10"
HUD_PATH = "/Game/Widgets/W_PlayerHUD"
ROOT_PATH = HUD_PATH + ".W_PlayerHUD:WidgetTree.CanvasPanel_54"
TREE_PATH = HUD_PATH + ".W_PlayerHUD:WidgetTree"

root = unreal.load_object(None, ROOT_PATH)
tree = unreal.load_object(None, TREE_PATH)
unreal.log(MARK + ": root=%s children=%d" % (str(root), root.get_children_count()))

existing = None
for i in range(root.get_children_count()):
    c = root.get_child_at(i)
    if c and c.get_name() == "GoldText":
        existing = c
if existing:
    unreal.log(MARK + ": GoldText already present, skipping")
else:
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
        unreal.log(MARK + ": font size failed: " + str(e))

    slot = root.add_child_to_canvas(tb)
    slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(1.0, 0.0), maximum=unreal.Vector2D(1.0, 0.0)))
    slot.set_alignment(unreal.Vector2D(1.0, 0.0))
    slot.set_position(unreal.Vector2D(-40.0, 40.0))
    slot.set_auto_size(True)
    slot.set_z_order(50)
    unreal.log(MARK + ": GoldText added, slot=" + str(slot))

wb = unreal.load_asset(HUD_PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(wb)
saved = unreal.EditorAssetLibrary.save_asset(HUD_PATH)
unreal.log(MARK + ": compiled + save -> " + str(saved))
