import unreal
p = '/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher'
bp = unreal.EditorAssetLibrary.load_asset(p)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(p)
print('SAVED', saved)

out = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir() + '_ai/t3d/GA_SkyCrusher_verify.T3D')
task = unreal.AssetExportTask()
task.set_editor_property('object', bp)
task.set_editor_property('filename', out)
task.set_editor_property('automated', True)
task.set_editor_property('replace_identical', True)
task.set_editor_property('prompt', False)
task.set_editor_property('write_empty_files', False)
ok = unreal.Exporter.run_asset_export_task(task)
print('EXPORT', ok, out)
