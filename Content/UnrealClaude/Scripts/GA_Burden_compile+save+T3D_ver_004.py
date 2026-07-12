
import unreal
bp = unreal.load_asset("/Game/GameplayAbilitySystem/Abilities/GA_Burden")
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
ok = unreal.EditorAssetLibrary.save_asset("/Game/GameplayAbilitySystem/Abilities/GA_Burden", only_if_is_dirty=False)
print("COMPILED+SAVED", ok)
out = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir() + "T3D2")
unreal.AssetToolsHelpers.get_asset_tools().export_assets(["/Game/GameplayAbilitySystem/Abilities/GA_Burden"], out)
print("EXPORTED", out)
