import unreal, json
res = {}

c = unreal.NarrativeInventoryComponent
cdo = unreal.get_default_object(c)
res["class_path"] = c.static_class().get_path_name()
res["parent"] = c.static_class().get_super_class().get_name()

# Editor-visible properties + current defaults, via T3D export of the CDO
try:
    res["export_text"] = cdo.export_text()[:3000]
except Exception as e:
    res["export_text"] = "ERR " + str(e)

# Probe likely property names
for p in ["capacity", "max_weight", "weight_capacity", "currency", "money",
          "inventory_capacity", "max_capacity", "starting_currency", "items"]:
    try:
        res["prop::" + p] = str(cdo.get_editor_property(p))
    except Exception as e:
        res["prop::" + p] = "ABSENT"

# Does any BP already own a NarrativeInventoryComponent?
res["existing_owners"] = []
for bp_path in ["/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer",
                "/Game/GameplayAbilitySystem/Characters/BP_NexusEnemy_Base"]:
    try:
        bp = unreal.EditorAssetLibrary.load_asset(bp_path)
        sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        for h in sds.k2_gather_subobject_data_for_blueprint(bp):
            d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
            o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
            if o and "NarrativeInventory" in o.get_class().get_name():
                res["existing_owners"].append(bp_path + " -> " + o.get_name())
    except Exception as e:
        res["existing_owners"].append(bp_path + " ERR " + str(e))

unreal.log("PS02_RESULT_BEGIN")
unreal.log(json.dumps(res, indent=2, default=str))
unreal.log("PS02_RESULT_END")
