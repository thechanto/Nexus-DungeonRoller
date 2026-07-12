import unreal
folder = "/Game/Inventory/Items"
name = "BP_Item_Weapon_ApprenticeStaff"
full = folder + "/" + name
if not unreal.EditorAssetLibrary.does_asset_exist(full):
    factory = unreal.NarrativeItemBlueprintFactory()
    parent = unreal.load_class(None, "/Script/NarrativeInventory.NarrativeItem")
    factory.set_editor_property("parent_class", parent)
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, None, factory)
    print("CREATED %r" % bp)
tex = unreal.EditorAssetLibrary.load_asset("/Game/Widgets/Images/TalentTree/crescent-staff")
assert tex, "crescent-staff texture missing"
cdo = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(full))
cdo.set_editor_property("display_name", "Apprentice's Staff")
cdo.set_editor_property("description",
    "A practice rod from a college that burned down with its students inside. "
    "It still hums their unfinished lessons.")
cdo.set_editor_property("weight", 5.0)
cdo.set_editor_property("stackable", False)
cdo.set_editor_property("thumbnail", tex)
ok = unreal.EditorAssetLibrary.save_asset(full)
cdo2 = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(full))
for prop in ["display_name", "description", "weight", "stackable", "thumbnail"]:
    print("STAFF %s = %r" % (prop, cdo2.get_editor_property(prop)))
print("STAFF_SAVED %s" % ok)
