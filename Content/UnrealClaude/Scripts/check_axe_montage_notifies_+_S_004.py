import unreal

out = unreal.Paths.project_dir() + "_ai/t3d"
at = unreal.AssetToolsHelpers.get_asset_tools()

# The known-working axe montage
res = unreal.AssetRegistryHelpers.get_asset_registry().get_assets_by_class(
    unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage"), True)
for a in res:
    n = str(a.asset_name)
    if "AxeSwing" in n or "AxeCombo" in n:
        unreal.log("FOUND %s" % a.get_full_name())
        at.export_assets([str(a.package_name) + "." + n], out)

# SkyCrusher damage config on the CDO
c = unreal.load_object(None, "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher.GA_SkyCrusher_C")
cdo = unreal.get_default_object(c)
for p in ["AttackMontage", "DamageEffectClass", "BaseDamage", "DamagePercentIncreasePerLevel"]:
    try:
        v = cdo.get_editor_property(p)
        unreal.log("SKY.%s = %s" % (p, v))
    except Exception as e:
        unreal.log("SKY.%s ERR %s" % (p, e))
unreal.log("DONE4")
