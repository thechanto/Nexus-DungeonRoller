import unreal

cls = unreal.load_class(None, "/Script/UMGEditor.UMGEditorProjectSettings")
print("[WRAP3] settings class =", cls)
s = unreal.get_default_object(cls)
print("[WRAP3] cdo =", s)
old_sel = s.get_editor_property("bUseWidgetTemplateSelector")
old_root = s.get_editor_property("DefaultRootWidget")
print("[WRAP3] before: selector=", old_sel, "root=", old_root)
s.set_editor_property("bUseWidgetTemplateSelector", False)
s.set_editor_property("DefaultRootWidget", unreal.CanvasPanel)

if unreal.EditorAssetLibrary.does_asset_exist("/Game/Widgets/W_NexusNarrativeHUD"):
    print("[WRAP3] deleted old:", unreal.EditorAssetLibrary.delete_asset("/Game/Widgets/W_NexusNarrativeHUD"))

factory = unreal.WidgetBlueprintFactory()
factory.set_editor_property("parent_class", unreal.UserWidget)
wb = unreal.AssetToolsHelpers.get_asset_tools().create_asset("W_NexusNarrativeHUD", "/Game/Widgets", unreal.WidgetBlueprint, factory)
print("[WRAP3] wb =", wb)

s.set_editor_property("bUseWidgetTemplateSelector", old_sel)
s.set_editor_property("DefaultRootWidget", old_root)

tree = unreal.load_object(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD:WidgetTree")
canvas = None
for name in ["CanvasPanel_0", "CanvasPanel", "RootWidget", "CanvasPanel_1"]:
    c = unreal.load_object(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD:WidgetTree.%s" % name)
    if c:
        canvas = c
        break
print("[WRAP3] canvas =", canvas)

hud_class = unreal.load_class(None, "/NarrativeCommonUI/Widgets/WBP_NarrativeHUD.WBP_NarrativeHUD_C")
hud = unreal.new_object(hud_class, outer=tree, name="NarrativeHUD")
slot = canvas.add_child_to_canvas(hud)
slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(0,0), maximum=unreal.Vector2D(1,1)))
slot.set_offsets(unreal.Margin(0,0,0,0))
print("[WRAP3] child added:", hud.get_name())

unreal.BlueprintEditorLibrary.compile_blueprint(wb)
print("[WRAP3] saved:", unreal.EditorAssetLibrary.save_asset("/Game/Widgets/W_NexusNarrativeHUD"))
