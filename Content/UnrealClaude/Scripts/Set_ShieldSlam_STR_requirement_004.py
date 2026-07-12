# @UnrealClaude Script
# @Name: UpdateAbilityRequirements
# @Description: Set ShieldSlam Strength requirement to 20 and ArcaneBolt Intelligence requirement to 20 in their ability data assets, then save both assets.
import re
import unreal

# (asset_path, stat name to match, known raw enum value fallback, new value)
TARGETS = [
    ("/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_ShieldSlam", "STRENGTH", 0, 20.0),
    ("/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_ArcaneBolt", "INTELLIGENCE", None, 20.0),
]

def mangled(struct_obj, friendly):
    """BP user-struct members have GUID-mangled FProperty names like
    Requirements_8_1234ABCD...; recover the exact name from export_text()."""
    txt = struct_obj.export_text()
    m = re.search(r'\b(%s(?:_\d+_[0-9A-Fa-f]{32})?)=' % re.escape(friendly), txt)
    if not m:
        raise RuntimeError("No member like '%s' in: %s" % (friendly, txt[:500]))
    return m.group(1)

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

    data = cdo.get_editor_property("AbilityData")
    reqs_prop = mangled(data, "Requirements")
    reqs = data.get_editor_property(reqs_prop)
    print("%s: %d requirement(s)" % (asset_name, len(reqs)))
    if len(reqs) == 0:
        raise RuntimeError("Empty requirements on " + asset_name)

    st_prop = mangled(reqs[0], "StatType")
    sv_prop = mangled(reqs[0], "StatValue")

    match_idx = -1
    for i in range(len(reqs)):
        st = reqs[i].get_editor_property(st_prop)
        sv = reqs[i].get_editor_property(sv_prop)
        print("  [%d] StatType=%s (raw=%s) StatValue=%s | export=%s" % (
            i, st, enum_int(st), sv, reqs[i].export_text()))
        if stat_key in str(st).upper():
            match_idx = i

    # Fallbacks if enum values print with internal (non-display) names
    if match_idx < 0:
        if len(reqs) == 1:
            match_idx = 0
            print("  name match failed; single requirement -> using index 0")
        elif raw_fallback is not None:
            for i in range(len(reqs)):
                if enum_int(reqs[i].get_editor_property(st_prop)) == raw_fallback:
                    match_idx = i
                    print("  name match failed; matched raw enum value %d -> index %d" % (raw_fallback, i))
                    break

    if match_idx < 0:
        raise RuntimeError("No %s requirement found on %s - aborting without saving" % (stat_key, asset_name))

    req = reqs[match_idx]
    req.set_editor_property(sv_prop, float(new_value))
    reqs[match_idx] = req
    data.set_editor_property(reqs_prop, reqs)
    cdo.set_editor_property("AbilityData", data)

    # Read back from the CDO to verify the write stuck before saving
    check = cdo.get_editor_property("AbilityData").get_editor_property(reqs_prop)[match_idx]
    print("  verify: StatType=%s StatValue=%s" % (
        check.get_editor_property(st_prop), check.get_editor_property(sv_prop)))

    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        raise RuntimeError("Failed to save " + asset_path)
    print("  -> %s requirement set to %s and asset saved" % (stat_key, new_value))

print("DONE")
