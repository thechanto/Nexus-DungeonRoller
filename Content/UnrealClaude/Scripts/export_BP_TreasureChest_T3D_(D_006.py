import unreal
bp = unreal.EditorAssetLibrary.load_asset("/Game/Interactables/BP_TreasureChest")
out = unreal.Paths.project_dir() + "T3D_Verify/BP_TreasureChest.T3D"
t = unreal.AssetExportTask()
t.set_editor_property("object", bp)
t.set_editor_property("filename", out)
t.set_editor_property("automated", True)
t.set_editor_property("prompt", False)
t.set_editor_property("replace_identical", True)
print("EXPORT:", unreal.Exporter.run_asset_export_task(t), out)
