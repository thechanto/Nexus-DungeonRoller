import unreal

# Save the input assets from the previous step first
for p in ["/Game/Input/Actions/IA_Inventory", "/Game/GameplayAbilitySystem/Input/IMC_AbilityInputMapping"]:
    ok = unreal.EditorAssetLibrary.save_asset(p)
    print("[WRAP] saved", p, ok)

# Create the wrapper widget blueprint
factory = unreal.WidgetBlueprintFactory()
factory.set_editor_property("parent_class", unreal.UserWidget)
at = unreal.AssetToolsHelpers.get_asset_tools()
existing = unreal.EditorAssetLibrary.does_asset_exist("/Game/Widgets/W_NexusNarrativeHUD")
print("[WRAP] exists already:", existing)
if not existing:
    wb = at.create_asset("W_NexusNarrativeHUD", "/Game/Widgets", unreal.WidgetBlueprint, factory)
else:
    wb = unreal.load_asset("/Game/Widgets/W_NexusNarrativeHUD")
print("[WRAP] wb =", wb)

tree = unreal.load_object(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD:WidgetTree")
print("[WRAP] tree =", tree)
root = tree.get_editor_property("root_widget")
print("[WRAP] existing root =", root)

if root is None or not isinstance(root, unreal.CanvasPanel):
    canvas = unreal.new_object(unreal.CanvasPanel, outer=tree, name="RootCanvas")
    tree.set_editor_property("root_widget", canvas)
else:
    canvas = root
print("[WRAP] canvas =", canvas)

hud_class = unreal.load_class(None, "/NarrativeCommonUI/Widgets/WBP_NarrativeHUD.WBP_NarrativeHUD_C")
print("[WRAP] hud_class =", hud_class)
hud = unreal.new_object(hud_class, outer=tree, name="NarrativeHUD")
slot = canvas.add_child_to_canvas(hud)
slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(0,0), maximum=unreal.Vector2D(1,1)))
slot.set_offsets(unreal.Margin(0,0,0,0))
print("[WRAP] child added:", hud.get_name(), "slot:", slot)

unreal.BlueprintEditorLibrary.compile_blueprint(wb)
ok = unreal.EditorAssetLibrary.save_asset("/Game/Widgets/W_NexusNarrativeHUD")
print("[WRAP] compiled+saved:", ok)
