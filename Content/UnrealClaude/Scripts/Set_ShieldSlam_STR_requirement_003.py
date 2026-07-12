# @UnrealClaude Script
# @Name: UpdateAbilityRequirements
# @Description: Set ShieldSlam Strength requirement to 20 and ArcaneBolt Intelligence requirement to 20 in their ability data assets, then save both assets.
import unreal

# (asset_path, stat name to match, known raw enum value fallback, new value)
TARGETS = [
    ("/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_ShieldSlam", "STRENGTH", 0, 20.0),
    ("/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_ArcaneBolt", "INTELLIGENCE", None, 20.0),
]

def resolve_prop(obj, name):
    """Find a property by friendly name, tolerating BP user-struct GUID-mangled names."""
    for n in (name, name.lower()):
        try:
            obj.get_editor_property(n)
            return n
        except Exception:
            pass
    key = name.lower().replace("_", "")
    for attr in dir(obj):
        if attr.startswith("__"):
            continue
        if attr.lower().replace("_", "").startswith(key):
            try:
                obj.get_editor_property(attr)
                return attr
            except Exception:
                continue
    raise RuntimeError("No property like '%s' on %s; attrs=%s" % (
        name, type(obj).__name__,
        [d for d in dir(obj) if not d.startswith("_")]))

def gep(obj, name):
    return obj.get_editor_property(resolve_prop(obj, name))

def sep(obj, name, value):
    obj.set_editor_property(resolve_prop(obj, name), value)

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

    data = gep(cdo, "AbilityData")
    reqs = gep(data, "Requirements")
    print("%s: %d requirement(s)" % (asset_name, len(reqs)))

    match_idx = -1
    for i in range(len(reqs)):
        st = gep(reqs[i], "StatType")
        sv = gep(reqs[i], "StatValue")
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
                if enum_int(gep(reqs[i], "StatType")) == raw_fallback:
                    match_idx = i
                    print("  name match failed; matched raw enum value %d -> index %d" % (raw_fallback, i))
                    break

    if match_idx < 0:
        raise RuntimeError("No %s requirement found on %s - aborting without saving" % (stat_key, asset_name))

    req = reqs[match_idx]
    sep(req, "StatValue", float(new_value))
    reqs[match_idx] = req
    sep(data, "Requirements", reqs)
    sep(cdo, "AbilityData", data)

    # Read back from the CDO to verify the write stuck before saving
    check = gep(gep(cdo, "AbilityData"), "Requirements")[match_idx]
    print("  verify: StatType=%s StatValue=%s" % (gep(check, "StatType"), gep(check, "StatValue")))

    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        raise RuntimeError("Failed to save " + asset_path)
    print("  -> %s requirement set to %s and asset saved" % (stat_key, new_value))

print("DONE")
