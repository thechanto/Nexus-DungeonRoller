
import unreal, os
out = r"C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/T3D_equip"
os.makedirs(out, exist_ok=True)
paths = ["/NarrativeInventory/NarrativeUI/WBP_EquippedItem","/NarrativeInventory/NarrativeUI/WBP_PlayerPreview"]
for p in paths:
    a = unreal.EditorAssetLibrary.load_asset(p)
    if not a: print("MISS",p); continue
    t = unreal.AssetExportTask()
    t.set_editor_property("object", a)
    t.set_editor_property("filename", os.path.join(out, a.get_name()+".t3d"))
    t.set_editor_property("automated", True); t.set_editor_property("prompt", False); t.set_editor_property("exporter", None)
    print("EXPORT", a.get_name(), unreal.Exporter.run_asset_export_task(t))
# Read DefaultThumbnail from the WBP_EquippedItem CDO
cls = unreal.EditorAssetLibrary.load_blueprint_class("/NarrativeInventory/NarrativeUI/WBP_EquippedItem")
if cls:
    cdo = unreal.get_default_object(cls)
    for prop in ["DefaultThumbnail","default_thumbnail","EquipmentSlot"]:
        try: print("CDO", prop, "=", cdo.get_editor_property(prop))
        except Exception as e: print("CDO", prop, "ERR", e)
print("DONE")
