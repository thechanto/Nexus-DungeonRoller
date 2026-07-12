
import unreal
# Edit the shared WBP_EquippedItem slot widget so empty slots render a visible frame.
bg = unreal.load_object(None, "/NarrativeInventory/NarrativeUI/WBP_EquippedItem.WBP_EquippedItem:WidgetTree.Image_BG")
print("BG loaded:", bg)
tex = unreal.load_asset("/NarrativeInventory/NarrativeUI/Assets/Icons/T_DefaultItemThumb")
print("TEX:", tex)
# Before state
b = bg.get_editor_property("brush")
print("BEFORE resource:", b.get_editor_property("resource_object"))
print("BEFORE color:", bg.get_editor_property("color_and_opacity"))
print("BEFORE renderopacity:", bg.get_editor_property("render_opacity"))
# Set a visible generic slot icon
b.set_editor_property("resource_object", tex)
b.set_editor_property("image_size", unreal.Vector2D(100.0,100.0))
bg.set_editor_property("brush", b)
bg.set_editor_property("color_and_opacity", unreal.LinearColor(1.0,1.0,1.0,0.55))
bg.set_editor_property("render_opacity", 1.0)
# Verify
b2 = bg.get_editor_property("brush")
print("AFTER resource:", b2.get_editor_property("resource_object"))
print("AFTER color:", bg.get_editor_property("color_and_opacity"))
print("AFTER renderopacity:", bg.get_editor_property("render_opacity"))
# Compile + save
bp = unreal.load_asset("/NarrativeInventory/NarrativeUI/WBP_EquippedItem")
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
ok = unreal.EditorAssetLibrary.save_asset("/NarrativeInventory/NarrativeUI/WBP_EquippedItem", only_if_is_dirty=False)
print("SAVE:", ok)
print("DONE")
