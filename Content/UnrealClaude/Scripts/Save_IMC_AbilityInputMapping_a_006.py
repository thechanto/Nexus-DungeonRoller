import unreal
for p in ["/Game/GameplayAbilitySystem/Input/IMC_AbilityInputMapping",
          "/Game/SavedGameData/BP_SaveGame"]:
    ok = unreal.EditorAssetLibrary.save_asset(p, only_if_is_dirty=False)
    unreal.log_warning("NEXUSSTEP12 saved {} -> {}".format(p, ok))
