
import unreal
path = "/Game/Interactables/BP_TreasureChest"
gen = unreal.load_class(None, path + ".BP_TreasureChest_C")
cdo = unreal.get_default_object(gen)
for n,v in [("MinGold",30),("MaxGold",80),("MinPotions",1),("MaxPotions",2)]:
    cdo.set_editor_property(n, v)
print("CDOFIX|PRE_SAVE", cdo.get_editor_property("MinGold"), cdo.get_editor_property("MaxGold"), cdo.get_editor_property("MinPotions"), cdo.get_editor_property("MaxPotions"))
ok = unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
print("CDOFIX|SAVED", ok)
# force reload from disk and re-read
unreal.EditorAssetLibrary.load_asset(path)
loaded = unreal.load_object(None, path)
unreal.EditorLoadingAndSavingUtils.reload_packages([loaded.get_outer()]) if False else None
# re-read CDO fresh
gen2 = unreal.load_class(None, path + ".BP_TreasureChest_C")
cdo2 = unreal.get_default_object(gen2)
print("CDOFIX|POST_RELOAD", cdo2.get_editor_property("MinGold"), cdo2.get_editor_property("MaxGold"), cdo2.get_editor_property("MinPotions"), cdo2.get_editor_property("MaxPotions"))
