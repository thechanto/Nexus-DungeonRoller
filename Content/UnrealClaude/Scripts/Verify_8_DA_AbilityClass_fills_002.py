import unreal
paths = [
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_ShieldSlam",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_SwordSweep",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_LeapSlash",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_SkyCrusher",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_ArcaneBolt",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_CosmicRift",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_Meteor",
 "/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_Burden",
]
member = "AbilityClass_24_B8BA6BBD4FAC1846FBBF2D9BB1DC4F80"
for p in paths:
    name = p.split('/')[-1]
    try:
        bp = unreal.EditorAssetLibrary.load_asset(p)
        cdo = unreal.get_default_object(bp.generated_class()) if isinstance(bp, unreal.Blueprint) else bp
        data = cdo.get_editor_property("AbilityData")
        val = data.get_editor_property(member)
        print("VERIFY2 %s -> %s" % (name, val))
    except Exception as e:
        print("VERIFY2 ERROR %s : %s" % (name, e))
print("VERIFY2 DONE")
