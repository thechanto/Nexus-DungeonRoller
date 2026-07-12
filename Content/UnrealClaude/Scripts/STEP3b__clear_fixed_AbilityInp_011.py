import unreal
eal = unreal.EditorAssetLibrary
res=[]
for name in ("GA_ArcaneBolt","GA_CosmicRift","GA_Meteor","GA_Burden"):
    p = "/Game/GameplayAbilitySystem/Abilities/%s" % name
    cls = eal.load_blueprint_class(p)
    cdo = unreal.get_default_object(cls)
    cdo.set_editor_property("ability_input_id", unreal.AbilityInputID.NONE)
    ok = eal.save_asset(p)
    res.append("%s input_id=%s saved=%s" % (name, cdo.get_editor_property("ability_input_id"), ok))
print("STEP3B_BEGIN")
for r in res: print(r)
print("STEP3B_END")