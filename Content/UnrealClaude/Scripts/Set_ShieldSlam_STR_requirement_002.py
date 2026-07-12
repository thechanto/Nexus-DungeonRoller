# @UnrealClaude Script
# @Name: UpdateAbilityRequirements
# @Description: Set ShieldSlam Strength requirement to 20 and ArcaneBolt Intelligence requirement to 20 in their ability data assets, then save both assets.
import unreal

# (asset_path, stat name to match, known raw enum value fallback, new value)
TARGETS = [
    ("/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_ShieldSlam", "STRENGTH", 0, 20.0),
    ("/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_ArcaneBolt", "INTELLIGENCE", None, 20.0),
]

def gep(obj, *names):
    last = None
    for n in names:
        try:
            return obj.get_editor_property(n)
        except Exception as e:
            last = e
    raise last

def sep(obj, value, *names):
    last = None
    for n in names:
        try:
            obj.set_editor_property(n, value)
            return
        except Exception as e:
            last = e
    raise last

def enum_int(v):
    try:
        return int(v)
    except Exception:
        return getattr(v, "value", None)

for asset_path, stat_key, raw_fallback, new_value in TARGETS:
    asset_name = asset_path.rsplit("/", 1)[-1]
    cls = unreal.load_object(None, "%s.%s_C" % (asset_path, asset_name))
    if not cls:
        raise RuntimeError("Failed to load class for " + asset_path)
    try:
        cdo = unreal.get_default_object(cls)
    except AttributeError:
        cdo = cls.get_default_object()

    data = gep(cdo, "AbilityData", "ability_data")
    reqs = gep(data, "Requirements", "requirements")
    print("%s: %d requirement(s)" % (asset_name, len(reqs)))

    match_idx = -1
    for i in range(len(reqs)):
        st = gep(reqs[i], "StatType", "stat_type")
        sv = gep(reqs[i], "StatValue", "stat_value")
        print("  [%d] StatType=%s (raw=%s) StatValue=%s" % (i, st, enum_int(st), sv))
        if stat_key in str(st).upper():
            match_idx = i

    # Fallbacks if enum entries print with internal (non-display) names
    if match_idx < 0:
        if len(reqs) == 1:
            match_idx = 0
            print("  name match failed; single requirement -> using index 0")
        elif raw_fallback is not None:
            for i in range(len(reqs)):
                if enum_int(gep(reqs[i], "StatType", "stat_type")) == raw_fallback:
                    match_idx = i
                    print("  name match failed; matched raw enum value %d -> index %d" % (raw_fallback, i))
                    break

    if match_idx < 0:
        raise RuntimeError("No %s requirement found on %s - aborting without saving" % (stat_key, asset_name))

    req = reqs[match_idx]
    sep(req, float(new_value), "StatValue", "stat_value")
    reqs[match_idx] = req
    sep(data, reqs, "Requirements", "requirements")
    sep(cdo, data, "AbilityData", "ability_data")

    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        raise RuntimeError("Failed to save " + asset_path)
    print("  -> %s requirement set to %s and asset saved" % (stat_key, new_value))

print("DONE")
