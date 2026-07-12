import unreal
# End PIE so we edit the editor assets, not the PIE-duplicated world.
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if les.is_in_play_in_editor():
    les.editor_request_end_play()

ABIL = "/Game/GameplayAbilitySystem/Abilities"
DA = "/Game/GameplayAbilitySystem/Abilities/DataAssets"
MANGLED = "AbilityClass_24_B8BA6BBD4FAC1846FBBF2D9BB1DC4F80"

pairs = [
    ("Warrior/DA_Ability_Warrior_ShieldSlam", "GA_ShieldSlam"),
    ("Warrior/DA_Ability_Warrior_SwordSweep", "GA_SwordSweep"),
    ("Warrior/DA_Ability_Warrior_LeapSlash",  "GA_LeapSlash"),
    ("Warrior/DA_Ability_Warrior_SkyCrusher", "GA_SkyCrusher"),
    ("Mage/DA_Ability_Mage_ArcaneBolt", "GA_ArcaneBolt"),
    ("Mage/DA_Ability_Mage_CosmicRift", "GA_CosmicRift"),
    ("Mage/DA_Ability_Mage_Meteor",     "GA_Meteor"),
    ("Mage/DA_Ability_Mage_Burden",     "GA_Burden"),
]

for da_rel, ga_name in pairs:
    da_short = da_rel.split("/")[-1]
    da_cls = unreal.load_class(None, "%s/%s.%s_C" % (DA, da_rel, da_short))
    ga_cls = unreal.load_class(None, "%s/%s.%s_C" % (ABIL, ga_name, ga_name))
    if not da_cls or not ga_cls:
        unreal.log_error("FILL missing: da=%s ga=%s" % (da_cls, ga_cls))
        continue
    ga_cdo = unreal.get_default_object(ga_cls)
    da_cdo = unreal.get_default_object(da_cls)
    struct = da_cdo.get_editor_property("AbilityData")   # copy
    try:
        struct.set_editor_property(MANGLED, ga_cdo)
    except Exception as e:
        unreal.log_error("FILL set mangled failed for %s: %s" % (da_short, e))
        try:
            struct.set_editor_property("AbilityClass", ga_cdo)
        except Exception as e2:
            unreal.log_error("FILL set display failed for %s: %s" % (da_short, e2))
            continue
    da_cdo.set_editor_property("AbilityData", struct)     # write chain back
    ok = unreal.EditorAssetLibrary.save_asset("%s/%s" % (DA, da_rel), only_if_is_dirty=False)
    # verify via re-read of the CDO struct
    verify = da_cdo.get_editor_property("AbilityData").export_text()
    has = MANGLED + "=" in verify and MANGLED + "=None" not in verify
    unreal.log("FILL %s <- %s save=%s verified=%s" % (da_short, ga_name, ok, has))

unreal.log("FILL_DONE")
