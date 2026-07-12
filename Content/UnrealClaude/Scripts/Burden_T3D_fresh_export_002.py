import unreal, os

out = unreal.Paths.project_saved_dir() + "T3D/"
out = unreal.Paths.convert_relative_path_to_full(out)
os.makedirs(out, exist_ok=True)

t = unreal.AssetExportTask()
t.set_editor_property("object", unreal.load_asset("/Game/GameplayAbilitySystem/Abilities/GA_Burden"))
t.set_editor_property("filename", out + "GA_Burden.T3D")
t.set_editor_property("automated", True)
t.set_editor_property("replace_identical", True)
t.set_editor_property("prompt", False)
t.set_editor_property("exporter", unreal.Exporter.get_default_export_object(unreal.Blueprint))
ok = unreal.Exporter.run_asset_export_task(t)
print("EXPORT_OK", ok, out + "GA_Burden.T3D")
