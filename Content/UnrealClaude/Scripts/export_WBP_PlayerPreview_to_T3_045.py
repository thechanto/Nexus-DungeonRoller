
import unreal, os
out = "C:/Users/chant/AppData/Local/Temp/ue_export"
os.makedirs(out, exist_ok=True)
a = unreal.EditorAssetLibrary.load_asset("/NarrativeInventory/NarrativeUI/WBP_PlayerPreview")
print("LOAD", a is not None)
if a:
    t = unreal.AssetExportTask()
    t.object = a; t.filename = os.path.join(out, a.get_name()+".T3D")
    t.automated = True; t.replace_identical = True; t.prompt = False
    unreal.Exporter.run_asset_export_task(t)
    print("EXPORT", os.path.exists(t.filename), os.path.getsize(t.filename) if os.path.exists(t.filename) else 0)
