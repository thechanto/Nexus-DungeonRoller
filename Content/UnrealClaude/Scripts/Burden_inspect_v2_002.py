import unreal, os, re

OUT = r"C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/T3D_Burden"
if not os.path.isdir(OUT):
    os.makedirs(OUT)

ASSETS = [
    "/Game/RetargetedAbilities/Mage/AM_Burden",
    "/Game/RetargetedAbilities/Mage/Burden_slow",
    "/Game/GameplayAbilitySystem/Abilities/GA_Burden",
    "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Base",
]

loaded = []
for p in ASSETS:
    a = unreal.EditorAssetLibrary.load_asset(p)
    print("LOAD %s -> %s" % (p, a))
    if a:
        loaded.append(a.get_path_name())

unreal.AssetToolsHelpers.get_asset_tools().export_assets(loaded, OUT)
print("EXPORTED to %s: %s" % (OUT, os.listdir(OUT)))

# --- montage facts straight from the object
m = unreal.EditorAssetLibrary.load_asset("/Game/RetargetedAbilities/Mage/AM_Burden")
if m:
    print("AM_Burden length      =", unreal.AnimationLibrary.get_sequence_length(m))
    print("AM_Burden rate scale  =", m.get_editor_property("rate_scale"))
    try:
        print("AM_Burden enable_root_motion =", m.get_editor_property("enable_root_motion"))
    except Exception as e:
        print("AM_Burden enable_root_motion : NOT EXPOSED (%s)" % e)
    try:
        tracks = unreal.AnimationLibrary.get_animation_notify_track_names(m)
        print("AM_Burden notify tracks =", tracks)
        for t in tracks:
            evs = unreal.AnimationLibrary.get_animation_notify_events(m, t)
            for e in evs:
                print("   NOTIFY track=%s time=%.4f  obj=%s" % (t, e.get_editor_property("trigger_time_offset") or 0.0, e))
    except Exception as e:
        print("notify api failed:", e)

src = unreal.EditorAssetLibrary.load_asset("/Game/RetargetedAbilities/Mage/Burden_slow")
if src:
    print("Burden_slow length =", unreal.AnimationLibrary.get_sequence_length(src))
    try:
        print("Burden_slow enable_root_motion =", src.get_editor_property("enable_root_motion"))
        print("Burden_slow root_motion_root_lock =", src.get_editor_property("root_motion_root_lock"))
    except Exception as e:
        print("src root motion props:", e)

# --- GA_Burden CDO
bc = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Abilities/GA_Burden")
print("GA_Burden class =", bc)
if bc:
    print("GA_Burden parent =", bc.get_super_class() if hasattr(bc, "get_super_class") else unreal.SystemLibrary.get_class_display_name(bc))
    cdo = unreal.get_default_object(bc)
    for prop in ["ability_tags", "activation_owned_tags", "montage_to_play", "damage", "projectile_class", "projectile_speed", "ability_input_id"]:
        try:
            print("  CDO %-22s = %s" % (prop, cdo.get_editor_property(prop)))
        except Exception as e:
            print("  CDO %-22s : (%s)" % (prop, e))

# --- keybind slots from the save
try:
    slots = unreal.NexusAbilityUILibrary.get_assigned_abilities()
    print("KEYBIND SLOTS =", slots)
except Exception as e:
    print("get_assigned_abilities failed:", e)

sg = r"C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/SaveGames"
print("SaveGames dir exists =", os.path.isdir(sg), os.listdir(sg) if os.path.isdir(sg) else "")
print("DONE")
