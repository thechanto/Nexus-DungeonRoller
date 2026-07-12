import unreal

cls = unreal.load_class(None, "/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy.BP_CameraShake_Hit_Enemy_C")
cdo = unreal.get_default_object(cls)
pat = cdo.get_editor_property("root_shake_pattern")
print("pattern: %s" % pat.get_class().get_name())

for a in ["location_amplitude_multiplier", "location_frequency_multiplier",
          "rotation_amplitude_multiplier", "rotation_frequency_multiplier"]:
    print("  %-32s %s" % (a, pat.get_editor_property(a)))

for a in ["x", "y", "z", "pitch", "yaw", "roll", "fov"]:
    v = pat.get_editor_property(a)
    print("  %-6s %s" % (a, v))
