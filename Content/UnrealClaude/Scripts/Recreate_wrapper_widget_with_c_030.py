import unreal

s = unreal.get_default_object(unreal.UMGEditorProjectSettings)
try:
    old_sel = s.get_editor_property("use_widget_template_selector")
    old_root = s.get_editor_property("default_root_widget")
    print("[WRAP2] settings before: selector=", old_sel, "root=", old_root)
except Exception as e:
    print("[WRAP2] settings probe FAILED:", e)
    raise

s.set_editor_property("use_widget_template_selector", False)
s.set_editor_property("default_root_widget", unreal.CanvasPanel)

# recreate cleanly
if unreal.EditorAssetLibrary.does_asset_exist("/Game/Widgets/W_NexusNarrativeHUD"):
    print("[WRAP2] deleting empty first attempt:", unreal.EditorAssetLibrary.delete_asset("/Game/Widgets/W_NexusNarrativeHUD"))

factory = unreal.WidgetBlueprintFactory()
factory.set_editor_property("parent_class", unreal.UserWidget)
wb = unreal.AssetToolsHelpers.get_asset_tools().create_asset("W_NexusNarrativeHUD", "/Game/Widgets", unreal.WidgetBlueprint, factory)
print("[WRAP2] wb =", wb)

# restore settings
s.set_editor_property("use_widget_template_selector", old_sel)
s.set_editor_property("default_root_widget", old_root)

tree = unreal.load_object(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD:WidgetTree")
canvas = None
for name in ["CanvasPanel_0", "RootCanvas", "CanvasPanel"]:
    canvas = unreal.load_object(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD:WidgetTree.%s" % name)
    if canvas: break
print("[WRAP2] canvas =", canvas)

hud_class = unreal.load_class(None, "/NarrativeCommonUI/Widgets/WBP_NarrativeHUD.WBP_NarrativeHUD_C")
hud = unreal.new_object(hud_class, outer=tree, name="NarrativeHUD")
slot = canvas.add_child_to_canvas(hud)
slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(0,0), maximum=unreal.Vector2D(1,1)))
slot.set_offsets(unreal.Margin(0,0,0,0))
print("[WRAP2] child added:", hud.get_name())

unreal.BlueprintEditorLibrary.compile_blueprint(wb)
print("[WRAP2] saved:", unreal.EditorAssetLibrary.save_asset("/Game/Widgets/W_NexusNarrativeHUD"))
