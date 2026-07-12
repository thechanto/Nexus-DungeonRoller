import unreal

print("=== MODULE STATE (class-resolution probe) ===")
probes = {
    "NarrativeInventory (Runtime)":        "/Script/NarrativeInventory.NarrativeInventoryComponent",
    "NarrativeEquipment (Runtime)":        "/Script/NarrativeEquipment.EquipmentComponent",
    "NarrativeInventoryEditor (Uncooked)": "/Script/NarrativeInventoryEditor.NarrativeItemBlueprint",
    "NarrativeEquipmentEditor (Uncooked)": "/Script/NarrativeEquipmentEditor.EquippableItemBlueprint",
    "CommonUI CommonActivatableWidget":    "/Script/CommonUI.CommonActivatableWidget",
    "CommonUI CommonButtonBase":           "/Script/CommonUI.CommonButtonBase",
}
for label, path in probes.items():
    try:
        c = unreal.load_object(None, path)
    except Exception:
        c = None
    print("  %-38s -> %s" % (label, "LOADED" if c else "*** NOT LOADED ***"))

print("")
print("=== /NarrativeCommonUI mount ===")
try:
    a = unreal.EditorAssetLibrary.list_assets("/NarrativeCommonUI", recursive=True)
    print("  assets found:", len(a))
except Exception as e:
    print("  list_assets failed:", e)
print("  does_asset_exist(WBP_NarrativeActivatableWidget) =",
      unreal.EditorAssetLibrary.does_asset_exist("/NarrativeCommonUI/Widgets/WBP_NarrativeActivatableWidget"))

print("")
print("=== ALL plugin WidgetBlueprints: parent + generated class ===")
ar = unreal.AssetRegistryHelpers.get_asset_registry()
assets = ar.get_assets_by_path("/NarrativeInventory", recursive=True)
wbps = [d for d in assets if str(d.asset_class_path.asset_name) == "WidgetBlueprint"]
print("  found %d WidgetBlueprints" % len(wbps))
print("")
for d in sorted(wbps, key=lambda x: str(x.asset_name)):
    name = str(d.asset_name)
    try:
        parent = str(d.get_tag_value("ParentClass"))
    except Exception:
        parent = "?"
    obj = unreal.load_object(None, "%s.%s" % (d.package_name, name))
    gen = None
    if obj:
        try:
            gen = obj.generated_class()
        except Exception:
            gen = None
    status = "OK    " if gen else "BROKEN"
    print("  [%s] %-30s parent=%s" % (status, name, parent))
