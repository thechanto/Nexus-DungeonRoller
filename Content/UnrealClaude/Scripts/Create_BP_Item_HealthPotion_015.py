import unreal
folder = "/Game/Inventory/Items"
name = "BP_Item_HealthPotion"
full = folder + "/" + name
if not unreal.EditorAssetLibrary.does_asset_exist(full):
    factory = unreal.NarrativeItemBlueprintFactory()
    parent = unreal.load_class(None, "/Script/NarrativeInventory.NarrativeItem")
    factory.set_editor_property("parent_class", parent)
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, None, factory)
    print("CREATED %r" % bp)
tex = unreal.EditorAssetLibrary.load_asset(
    "/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_1_1024x1024")
assert tex, "potion texture missing"
cdo = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(full))
cdo.set_editor_property("display_name", "Health Potion")
cdo.set_editor_property("description",
    "A draught of clotted crimson that beats faintly against the glass. "
    "Drink it and your flask remembers what it is to be full.")
cdo.set_editor_property("weight", 0.5)
cdo.set_editor_property("stackable", True)
cdo.set_editor_property("max_stack_size", 10)
cdo.set_editor_property("consume_on_use", True)
cdo.set_editor_property("use_action_text", "Drink")
cdo.set_editor_property("thumbnail", tex)
ok = unreal.EditorAssetLibrary.save_asset(full)
cdo2 = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(full))
for prop in ["display_name", "description", "weight", "stackable", "max_stack_size",
             "consume_on_use", "use_action_text", "thumbnail"]:
    print("POTION %s = %r" % (prop, cdo2.get_editor_property(prop)))
print("POTION_SAVED %s" % ok)
