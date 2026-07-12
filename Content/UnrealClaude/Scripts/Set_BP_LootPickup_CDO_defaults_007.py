import unreal

MARK = "LOOT06"
BP_PATH = "/Game/Loot/BP_LootPickup"
cls = unreal.load_object(None, BP_PATH + ".BP_LootPickup_C")
cdo = unreal.get_default_object(cls)
cdo.set_editor_property("bIsGold", True)
cdo.set_editor_property("GoldAmount", 10)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH)

# re-read to verify
cdo2 = unreal.get_default_object(unreal.load_object(None, BP_PATH + ".BP_LootPickup_C"))
unreal.log(MARK + ": bIsGold=%s GoldAmount=%s PickupSound=%s save=%s" % (
    cdo2.get_editor_property("bIsGold"), cdo2.get_editor_property("GoldAmount"),
    cdo2.get_editor_property("PickupSound"), saved))
