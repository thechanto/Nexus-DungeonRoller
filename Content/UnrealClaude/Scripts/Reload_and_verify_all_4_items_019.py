import unreal
items = ["BP_Item_Gold", "BP_Item_HealthPotion", "BP_Item_Weapon_RustedAxe", "BP_Item_Weapon_ApprenticeStaff"]
paths = ["/Game/Inventory/Items/" + n for n in items]
pkgs = [unreal.load_package(p) for p in paths]
results = unreal.EditorLoadingAndSavingUtils.reload_packages(pkgs)
print("RELOADED %s" % (results,))
props = ["display_name", "description", "weight", "stackable", "max_stack_size",
         "consume_on_use", "use_action_text", "thumbnail"]
for p in paths:
    cls = unreal.EditorAssetLibrary.load_blueprint_class(p)
    cdo = unreal.get_default_object(cls)
    line = []
    for prop in props:
        try:
            line.append("%s=%r" % (prop, cdo.get_editor_property(prop)))
        except Exception as e:
            line.append("%s=ERR" % prop)
    print("ITEM %s parent=%s :: %s" % (p.split("/")[-1], cls.get_class_path_name(), " | ".join(line)))
print("VERIFY_ALL_DONE")
