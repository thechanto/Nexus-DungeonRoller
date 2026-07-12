import unreal
ok = unreal.EditorAssetLibrary.save_asset("/Game/SavedGameData/BP_SaveGame")
unreal.log("SAVE_RESULT: " + str(ok))