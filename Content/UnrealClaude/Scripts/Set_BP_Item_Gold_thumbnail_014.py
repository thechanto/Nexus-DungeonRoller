import unreal
p = "/Game/Inventory/Items/BP_Item_Gold"
tex = unreal.EditorAssetLibrary.load_asset("/Game/StarterContent/Textures/T_Metal_Gold_D")
assert tex, "texture missing"
cls = unreal.EditorAssetLibrary.load_blueprint_class(p)
cdo = unreal.get_default_object(cls)
cdo.set_editor_property("thumbnail", tex)
ok = unreal.EditorAssetLibrary.save_asset(p)
cdo2 = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(p))
print("GOLD_THUMB saved=%s thumbnail=%r" % (ok, cdo2.get_editor_property("thumbnail")))
