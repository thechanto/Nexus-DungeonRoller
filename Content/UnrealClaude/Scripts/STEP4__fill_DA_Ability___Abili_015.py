import unreal

eal = unreal.EditorAssetLibrary
res = []
MANGLED = "AbilityClass_24_B8BA6BBD4FAC1846FBBF2D9BB1DC4F80"
GA = "/Game/GameplayAbilitySystem/Abilities/"
DA = "/Game/GameplayAbilitySystem/Abilities/DataAssets/"

mapping = [
    (DA + "Warrior/DA_Ability_Warrior_ShieldSlam", GA + "GA_ShieldSlam"),
    (DA + "Warrior/DA_Ability_Warrior_SwordSweep", GA + "GA_SwordSweep"),
    (DA + "Warrior/DA_Ability_Warrior_LeapSlash",  GA + "GA_LeapSlash"),
    (DA + "Warrior/DA_Ability_Warrior_SkyCrusher", GA + "GA_SkyCrusher"),
    (DA + "Mage/DA_Ability_Mage_ArcaneBolt",       GA + "GA_ArcaneBolt"),
    (DA + "Mage/DA_Ability_Mage_CosmicRift",       GA + "GA_CosmicRift"),
    (DA + "Mage/DA_Ability_Mage_Meteor",           GA + "GA_Meteor"),
    (DA + "Mage/DA_Ability_Mage_Burden",           GA + "GA_Burden"),
]

for da_path, ga_path in mapping:
    try:
        ga_class = eal.load_blueprint_class(ga_path)
        if ga_class is None:
            res.append("FAIL %s: GA class not found %s" % (da_path, ga_path))
            continue
        ga_cdo = unreal.get_default_object(ga_class)

        da_class = eal.load_blueprint_class(da_path)
        da_cdo = unreal.get_default_object(da_class)
        data = da_cdo.get_editor_property("AbilityData")
        data.set_editor_property(MANGLED, ga_cdo)
        da_cdo.set_editor_property("AbilityData", data)

        readback = da_cdo.get_editor_property("AbilityData").get_editor_property(MANGLED)
        saved = eal.save_asset(da_path)
        rb_name = readback.get_class().get_name() if readback is not None else "None"
        res.append("SET %s AbilityClass=%s saved=%s" % (da_path.split("/")[-1], rb_name, saved))
    except Exception as e:
        res.append("ERROR %s: %s" % (da_path, str(e)[:200]))

print("STEP4_BEGIN")
for r in res:
    print(r)
print("STEP4_END")
