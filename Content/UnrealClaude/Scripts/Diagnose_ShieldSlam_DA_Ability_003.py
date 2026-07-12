import unreal
c = unreal.load_class(None, "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_ShieldSlam.DA_Ability_Warrior_ShieldSlam_C")
cdo = unreal.get_default_object(c)
data = cdo.get_editor_property("AbilityData")
unreal.log("DIAG export: %s" % data.export_text())
unreal.log("DIAG_DONE")
