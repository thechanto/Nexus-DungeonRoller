import unreal
paths = [
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_ShieldSlam",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_SwordSweep",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_LeapSlash",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_SkyCrusher",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_ArcaneBolt",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_CosmicRift",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_Meteor",
 "/Game/GameplayAbilitySystem/AbilityData/DA_Ability_Burden",
]
member = "AbilityClass_24_B8BA6BBD4FAC1846FBBF2D9BB1DC4F80"
for p in paths:
    if not unreal.EditorAssetLibrary.does_asset_exist(p):
        print("VERIFY MISSING_PATH %s" % p)
        continue
    da = unreal.EditorAssetLibrary.load_asset(p)
    cdo = unreal.get_default_object(da.generated_class()) if isinstance(da, unreal.Blueprint) else da
    try:
        data = cdo.get_editor_property("AbilityData")
        val = data.get_editor_property(member)
        print("VERIFY %s -> %s" % (p.split('/')[-1], val))
    except Exception as e:
        print("VERIFY ERROR %s : %s" % (p.split('/')[-1], e))
print("VERIFY DONE")
