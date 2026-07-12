import unreal
bp = unreal.EditorAssetLibrary.load_asset("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher")
t = unreal.AssetExportTask()
t.set_editor_property("object", bp)
t.set_editor_property("filename", unreal.Paths.project_dir() + "T3D_Verify/GA_SkyCrusher_final.T3D")
t.set_editor_property("automated", True); t.set_editor_property("prompt", False); t.set_editor_property("replace_identical", True)
unreal.Exporter.run_asset_export_task(t)
