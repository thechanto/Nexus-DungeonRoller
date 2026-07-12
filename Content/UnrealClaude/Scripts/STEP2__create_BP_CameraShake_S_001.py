import unreal

PKG = "/Game/VFX/CameraShakes"
NAME = "BP_CameraShake_SkyCrusher"
FULL = PKG + "/" + NAME

tools = unreal.AssetToolsHelpers.get_asset_tools()

if not unreal.EditorAssetLibrary.does_asset_exist(FULL):
    fac = unreal.BlueprintFactory()
    fac.set_editor_property("parent_class", unreal.DefaultCameraShakeBase)
    bp = tools.create_asset(NAME, PKG, unreal.Blueprint, fac)
    print("created: %s" % bp)
else:
    print("already exists")

cls = unreal.load_class(None, FULL + "." + NAME + "_C")
cdo = unreal.get_default_object(cls)
pat = cdo.get_editor_property("root_shake_pattern")
print("pattern: %s" % (pat.get_class().get_name() if pat else None))


def set_osc(prop, amp, freq):
    o = pat.get_editor_property(prop)   # struct copy
    o.set_editor_property("amplitude", amp)
    o.set_editor_property("frequency", freq)
    pat.set_editor_property(prop, o)    # write whole struct back


# Heavy slam. Reference BP_CameraShake_Hit_Enemy = locAmp 2 / locFreq 10 / rotAmp 0, z-amp 3.
pat.set_editor_property("location_amplitude_multiplier", 12.0)
pat.set_editor_property("location_frequency_multiplier", 14.0)
pat.set_editor_property("rotation_amplitude_multiplier", 4.0)
pat.set_editor_property("rotation_frequency_multiplier", 12.0)

set_osc("x", 1.0, 1.0)
set_osc("y", 1.0, 1.0)
set_osc("z", 4.0, 1.0)     # vertical dominates -> reads as a ground slam
set_osc("pitch", 1.5, 1.0)
set_osc("yaw", 1.0, 1.0)
set_osc("roll", 2.0, 1.0)
set_osc("fov", 0.0, 1.0)

print("--- readback ---")
for a in ["location_amplitude_multiplier", "location_frequency_multiplier",
          "rotation_amplitude_multiplier", "rotation_frequency_multiplier"]:
    print("  %-32s %s" % (a, pat.get_editor_property(a)))
for a in ["x", "y", "z", "pitch", "yaw", "roll"]:
    o = pat.get_editor_property(a)
    print("  %-6s amp=%.2f freq=%.2f" % (a, o.amplitude, o.frequency))

print("saved=%s" % unreal.EditorAssetLibrary.save_asset(FULL, only_if_is_dirty=False))
