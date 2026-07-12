import unreal
ok = unreal.EditorAssetLibrary.save_asset("/Game/Interactables/BP_TreasureChest", only_if_is_dirty=False)
print("SAVE_CHEST2:", ok)
