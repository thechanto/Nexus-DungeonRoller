
import unreal, os
out = "C:/Users/chant/AppData/Local/Temp/ue_export"
os.makedirs(out, exist_ok=True)
paths = [
    "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory",
    "/NarrativeInventory/NarrativeUI/WBP_EquippedItem",
    "/NarrativeInventory/NarrativeUI/WBP_InventoryWidget",
]
tools = unreal.AssetToolsHelpers.get_asset_tools()
loaded = []
for p in paths:
    a = unreal.EditorAssetLibrary.load_asset(p)
    if a: loaded.append(a)
    print("LOAD", p, a is not None)
task = unreal.AssetExportTask()
for a in loaded:
    t = unreal.AssetExportTask()
    t.object = a
    t.filename = os.path.join(out, a.get_name()+".T3D")
    t.automated = True
    t.replace_identical = True
    t.prompt = False
    unreal.Exporter.run_asset_export_task(t)
    print("EXPORT", a.get_name(), os.path.exists(t.filename), os.path.getsize(t.filename) if os.path.exists(t.filename) else 0)
