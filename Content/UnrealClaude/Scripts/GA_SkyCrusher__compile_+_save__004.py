import unreal

BP_PATH = "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher"
bp = unreal.EditorAssetLibrary.load_asset(BP_PATH)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
print("COMPILED")

saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
print("SAVED:", saved)

out_dir = unreal.Paths.project_dir() + "T3D_Verify/"
task = unreal.AssetExportTask()
task.set_editor_property("object", bp)
task.set_editor_property("filename", out_dir + "GA_SkyCrusher.T3D")
task.set_editor_property("automated", True)
task.set_editor_property("prompt", False)
task.set_editor_property("replace_identical", True)
ok = unreal.Exporter.run_asset_export_task(task)
print("T3D_EXPORT:", ok, "->", out_dir + "GA_SkyCrusher.T3D")
