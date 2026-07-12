
import unreal
out = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir() + "T3D")
unreal.AssetToolsHelpers.get_asset_tools().export_assets(["/Game/GameplayAbilitySystem/Abilities/GA_Burden"], out)
print("DONE", out)
