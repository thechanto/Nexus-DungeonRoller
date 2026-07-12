import unreal
p = "/Game/Inventory/Items/BP_Item_Gold"
cls = unreal.EditorAssetLibrary.load_blueprint_class(p)
cdo = unreal.get_default_object(cls)
for prop in ["display_name", "description", "weight", "stackable", "max_stack_size",
             "thumbnail", "base_value", "consume_on_use", "use_action_text", "pickup_mesh"]:
    try:
        print("GOLD %s = %r" % (prop, cdo.get_editor_property(prop)))
    except Exception as e:
        print("GOLD NOPROP %s (%s)" % (prop, e))
print("GOLD_VERIFY_DONE")
