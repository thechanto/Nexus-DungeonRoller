import unreal
out = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/T3D"
paths = ["/Game/GameplayAbilitySystem/Abilities/GA_Burden",
         "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher"]
unreal.AssetToolsHelpers.get_asset_tools().export_assets(paths, out)
unreal.log("EXPORT_DONE")
