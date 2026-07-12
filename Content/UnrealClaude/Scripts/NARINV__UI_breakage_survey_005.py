import unreal

print("=== Mounted content roots ===")
for r in sorted(unreal.Paths.get_mounted_content_directories() if hasattr(unreal.Paths, "get_mounted_content_directories") else []):
    print("  ", r)

print("")
print("=== Does /NarrativeCommonUI/ exist? ===")
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for root in ["/NarrativeCommonUI", "/NarrativeInventory", "/Game"]:
    try:
        assets = ar.get_assets_by_path(root, recursive=True)
        print("  %-22s %d assets" % (root, len(assets)))
    except Exception as e:
        print("  %-22s ERR %s" % (root, e))

print("")
print("=== Which plugin UI widgets actually load? ===")
widgets = [
 "/NarrativeInventory/NarrativeUI/WBP_InventoryWidget",
 "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory",
 "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting",
 "/NarrativeInventory/NarrativeUI/Looting/WBP_Loot_YourInventory",
 "/NarrativeInventory/NarrativeUI/Looting/WBP_Loot_TheirInventory",
 "/NarrativeInventory/NarrativeUI/WBP_Item",
 "/NarrativeInventory/NarrativeUI/WBP_ItemTooltip",
 "/NarrativeInventory/NarrativeUI/WBP_ItemThumbnail",
 "/NarrativeInventory/NarrativeUI/WBP_ItemInspector",
 "/NarrativeInventory/NarrativeUI/WBP_EquippedItem",
 "/NarrativeInventory/NarrativeUI/WBP_ItemDragVisual",
 "/NarrativeInventory/NarrativeUI/WBP_PlayerPreview",
 "/NarrativeInventory/NarrativeUI/WBP_ItemStat",
 "/NarrativeInventory/NarrativeUI/InventoryFilters/WBP_InventoryFilter",
]
ok, bad = [], []
for w in widgets:
    s = w.split("/")[-1]
    try:
        c = unreal.load_class(None, w + "." + s + "_C")
        (ok if c else bad).append(s)
    except Exception:
        bad.append(s)
print("  LOADS OK  (%d): %s" % (len(ok), ", ".join(ok)))
print("  BROKEN    (%d): %s" % (len(bad), ", ".join(bad)))

print("")
print("=== Non-UI core still fine? ===")
for c in ["/Script/NarrativeInventory.NarrativeInventoryComponent",
          "/Script/NarrativeEquipment.EquipmentComponent",
          "/NarrativeInventory/Misc/ExampleItems/BI_ExampleItemA.BI_ExampleItemA_C",
          "/NarrativeInventory/Blueprints/BP_BasicItemPickup.BP_BasicItemPickup_C"]:
    try:
        o = unreal.load_object(None, c) or unreal.load_class(None, c)
        print("  %-64s %s" % (c.split("/")[-1], "OK" if o else "NULL"))
    except Exception as e:
        print("  %-64s FAIL" % c.split("/")[-1])
