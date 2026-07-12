import unreal

BP = "/Game/Widgets/W_LockOnIndicator"
wb = unreal.load_asset(BP)
tree = unreal.load_object(None, BP + ".W_LockOnIndicator:WidgetTree")
sizebox = unreal.load_object(None, BP + ".W_LockOnIndicator:WidgetTree.SizeBox_14")
tb = unreal.load_object(None, BP + ".W_LockOnIndicator:WidgetTree.TextBlock_79")
unreal.log("LOCKON05: loaded tree=%s sizebox=%s tb=%s" % (bool(tree), bool(sizebox), bool(tb)))

# swap content: TextBlock out, gold crosshair Image in
sizebox.remove_child(tb)
tex = unreal.load_asset("/Game/Widgets/Textures/T_Crosshair_Dot")
img = unreal.new_object(unreal.Image, outer=tree, name="LockOnImage")
try:
    brush = img.get_editor_property("brush")
    brush.set_editor_property("resource_object", tex)
    img.set_editor_property("brush", brush)
    unreal.log("LOCKON05: brush set")
except Exception as e:
    unreal.log("LOCKON05: brush set FAILED " + str(e))
img.set_editor_property("color_and_opacity", unreal.LinearColor(0.788, 0.647, 0.361, 1.0))  # C9A55C gold
added = sizebox.add_child(img)
unreal.log("LOCKON05: image added slot=" + str(added))

sizebox.set_editor_property("width_override", 70.0)
sizebox.set_editor_property("height_override", 70.0)

unreal.BlueprintEditorLibrary.compile_blueprint(wb)
ok = unreal.EditorAssetLibrary.save_asset(BP, only_if_is_dirty=False)
unreal.log("LOCKON05: compiled, saved=" + str(ok))
