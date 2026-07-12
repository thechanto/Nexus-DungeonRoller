import unreal
bp = "/Game/Interactables/BP_TreasureChest"
ok = unreal.EditorAssetLibrary.save_asset(bp, only_if_is_dirty=False)
print("SAVE_CHEST_RESULT:", ok)
