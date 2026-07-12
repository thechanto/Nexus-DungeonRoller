import unreal

ar = unreal.AssetRegistryHelpers.get_asset_registry()

names = [
    "/NarrativeInventory/NarrativeUI/WBP_InventoryWidget",
    "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory",
    "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting",
    "/NarrativeInventory/NarrativeUI/WBP_Item",
    "/NarrativeInventory/NarrativeUI/WBP_PlayerPreview",
    "/NarrativeInventory/Blueprints/BP_BasicItemPickup",
]

print("=== ParentClass via AssetRegistry tags ===")
for n in names:
    short = n.split("/")[-1]
    try:
        ad = ar.get_asset_by_object_path(n + "." + short)
        tag = ad.get_tag_value("ParentClass") if ad else None
        print("%-28s -> %s" % (short, tag))
    except Exception as e:
        print("%-28s -> ERR %s" % (short, e))

print("")
print("=== Generated class (_C) loads? ===")
for n in names:
    short = n.split("/")[-1]
    try:
        c = unreal.load_class(None, n + "." + short + "_C")
        print("%-28s -> %s" % (short, "OK" if c else "NULL"))
    except Exception as e:
        print("%-28s -> FAILED (%s)" % (short, type(e).__name__))
