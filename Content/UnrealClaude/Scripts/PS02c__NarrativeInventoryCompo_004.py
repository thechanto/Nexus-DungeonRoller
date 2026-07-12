import unreal, json
res = {}

c = unreal.NarrativeInventoryComponent
cdo = unreal.get_default_object(c)
res["class_path"] = c.static_class().get_path_name()
res["mro"] = [b.__name__ for b in c.__mro__[:5]]

try:
    res["export_text"] = cdo.export_text()
except Exception as e:
    res["export_text"] = "ERR " + str(e)

for p in ["capacity", "max_weight", "weight_capacity", "currency", "money",
          "inventory_capacity", "max_capacity", "starting_currency", "items"]:
    try:
        res["prop::" + p] = str(cdo.get_editor_property(p))
    except Exception:
        res["prop::" + p] = "ABSENT"

res["existing_owners"] = []
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
for bp_path in ["/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer",
                "/Game/GameplayAbilitySystem/Characters/BP_NexusEnemy_Base"]:
    try:
        bp = unreal.EditorAssetLibrary.load_asset(bp_path)
        for h in sds.k2_gather_subobject_data_for_blueprint(bp):
            d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
            o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
            if o and "NarrativeInventory" in o.get_class().get_name():
                res["existing_owners"].append(bp_path + " -> " + o.get_name())
    except Exception as e:
        res["existing_owners"].append(bp_path + " ERR " + str(e))

out = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/ClaudeScripts/PS02_out.json"
with open(out, "w") as f:
    json.dump(res, f, indent=2, default=str)
unreal.log("PS02 wrote " + out)
