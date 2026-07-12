import unreal

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
res = []

PKG = "/Game/GameplayAbilitySystem/Abilities"
melee_parent = eal.load_blueprint_class(PKG + "/GA_MeleeAttack_Base")
proj_parent = eal.load_blueprint_class(PKG + "/GA_ShootProjectile_Base")
ice_projectile = eal.load_blueprint_class("/Game/Projectiles/BP_Projectile_Ice")

def make_factory(parent):
    try:
        f = unreal.GameplayAbilitiesBlueprintFactory()
    except Exception:
        f = unreal.BlueprintFactory()
    f.set_editor_property("parent_class", parent)
    return f

def create_ga(name, parent, props):
    full = "%s/%s" % (PKG, name)
    if eal.does_asset_exist(full):
        bp = eal.load_asset(full)
        res.append("EXISTS %s" % full)
    else:
        bp = at.create_asset(name, PKG, None, make_factory(parent))
        if bp is None:
            res.append("FAIL_CREATE %s" % full)
            return
        res.append("CREATED %s (%s)" % (full, bp.get_class().get_name()))
    cls = eal.load_blueprint_class(full)
    cdo = unreal.get_default_object(cls)
    for pname, val in props.items():
        try:
            cdo.set_editor_property(pname, val)
            rb = cdo.get_editor_property(pname)
            res.append("  %s = %s" % (pname, rb))
        except Exception as e:
            res.append("  %s SET_ERR %s" % (pname, str(e)[:120]))
    saved = eal.save_asset(full)
    res.append("  saved=%s" % saved)

def load_montage(p):
    return eal.load_asset(p)

W = "/Game/RetargetedAbilities/Warrior/"
M = "/Game/RetargetedAbilities/Mage/"

melee_abilities = [
    ("GA_ShieldSlam", W + "AM_ShieldSlam", 25.0),
    ("GA_SwordSweep", W + "AM_SwordSweep", 35.0),
    ("GA_LeapSlash",  W + "AM_LeapSlash",  50.0),
    ("GA_SkyCrusher", W + "AM_SkyCrusher", 80.0),
]
for name, mpath, dmg in melee_abilities:
    create_ga(name, melee_parent, {
        "AttackMontage": load_montage(mpath),
        "BaseDamage": dmg,
        "DamagePercentIncreasePerLevel": 0.5,
    })

ranged_abilities = [
    ("GA_ArcaneBolt", M + "AM_ArcaneBolt", 25.0),
    ("GA_CosmicRift", M + "AM_CosmicRift", 35.0),
    ("GA_Meteor",     M + "AM_Meteor",     50.0),
    ("GA_Burden",     M + "AM_Burden",     80.0),
]
for name, mpath, dmg in ranged_abilities:
    create_ga(name, proj_parent, {
        "ShootMontage": load_montage(mpath),
        "DamageMagnitude": dmg,
        "ProjectileClass": ice_projectile,
        "ProjectileSpeed": 2000.0,
        "DamagePercentIncreasePerLevel": 0.2,
    })

# sanity: confirm inherited input/UI flags on one melee and one ranged child
for name in ("GA_ShieldSlam", "GA_ArcaneBolt"):
    cls = eal.load_blueprint_class("%s/%s" % (PKG, name))
    cdo = unreal.get_default_object(cls)
    res.append("CHECK %s input_id=%s show_in_bar=%s auto_activate=%s tags=%s" % (
        name,
        cdo.get_editor_property("ability_input_id"),
        cdo.get_editor_property("should_show_in_abilities_bar"),
        cdo.get_editor_property("auto_activate_when_granted"),
        cdo.get_editor_property("ability_tags")))

print("STEP3_BEGIN")
for r in res:
    print(r)
print("STEP3_END")
