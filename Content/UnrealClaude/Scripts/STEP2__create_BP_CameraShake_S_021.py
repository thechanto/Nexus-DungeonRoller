import unreal

SRC = "/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy"
DST = "/Game/VFX/CameraShakes/BP_CameraShake_SkyCrusher"

if unreal.EditorAssetLibrary.does_asset_exist(DST):
    print("exists already; reusing")
else:
    ok = unreal.EditorAssetLibrary.duplicate_asset(SRC, DST)
    print("duplicated -> %s" % ok)

cls = unreal.load_class(None, DST + "." + DST.split("/")[-1] + "_C")
cdo = unreal.get_default_object(cls)
pat = cdo.get_editor_property("root_shake_pattern")
print("pattern: %s" % pat.get_class().get_name())


def set_osc(prop, amp, freq):
    o = pat.get_editor_property(prop)      # struct copy
    o.set_editor_property("amplitude", amp)
    o.set_editor_property("frequency", freq)
    pat.set_editor_property(prop, o)       # write the whole struct back


# Heavy slam: big vertical punch + rotational kick (the hit shake has rotAmp=0)
pat.set_editor_property("location_amplitude_multiplier", 12.0)
pat.set_editor_property("location_frequency_multiplier", 14.0)
pat.set_editor_property("rotation_amplitude_multiplier", 4.0)
pat.set_editor_property("rotation_frequency_multiplier", 12.0)

set_osc("x", 1.0, 1.0)
set_osc("y", 1.0, 1.0)
set_osc("z", 4.0, 1.0)      # vertical dominates -> reads as a ground slam
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

print("saved=%s" % unreal.EditorAssetLibrary.save_asset(DST, only_if_is_dirty=False))
