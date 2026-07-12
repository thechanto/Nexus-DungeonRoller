
import unreal
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for nm in ["WBP_EquippedItem","WBP_PlayerPreview"]:
    print("==", nm)
    hits = ar.get_assets_by_package_name(nm) if False else None
    # search by class
for cls in ["WidgetBlueprint"]:
    pass
# brute: enumerate all assets whose name matches
opts = unreal.AssetRegistryDependencyOptions()
allassets = ar.get_all_assets(include_only_on_disk_assets=False)
want = ("WBP_EquippedItem","WBP_PlayerPreview")
for a in allassets:
    n = str(a.asset_name)
    if n in want:
        print("FOUND", n, "->", str(a.package_name))
print("DONE")
