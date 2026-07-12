
import unreal, os
out = r"C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/T3D_equip"
os.makedirs(out, exist_ok=True)
paths = [
    "/NarrativeInventory/NarrativeUI/Inventory/WBP_EquippedItem",
    "/NarrativeInventory/NarrativeUI/Inventory/WBP_PlayerPreview",
]
for p in paths:
    a = unreal.EditorAssetLibrary.load_asset(p)
    if not a:
        print("MISS", p); continue
    t = unreal.AssetExportTask()
    t.set_editor_property("object", a)
    t.set_editor_property("filename", os.path.join(out, a.get_name()+".t3d"))
    t.set_editor_property("automated", True)
    t.set_editor_property("prompt", False)
    t.set_editor_property("exporter", None)
    ok = unreal.Exporter.run_asset_export_task(t)
    print("EXPORT", a.get_name(), ok)
print("DONE")
