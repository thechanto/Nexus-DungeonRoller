import unreal

cls = unreal.load_class(None, "/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy.BP_CameraShake_Hit_Enemy_C")
cdo = unreal.get_default_object(cls)
pat = cdo.get_editor_property("root_shake_pattern")

print("pattern: %s" % pat.get_class().get_name())
attrs = [a for a in dir(pat) if not a.startswith("_") and a not in
         ("cast", "copy", "get_class", "get_default_object", "get_editor_property",
          "get_fname", "get_full_name", "get_name", "get_outer", "get_outermost",
          "get_path_name", "get_typed_outer", "get_world", "modify", "rename",
          "set_editor_properties", "set_editor_property", "static_class")]
print("attrs: %s" % attrs)

for a in sorted(attrs):
    try:
        v = pat.get_editor_property(a)
    except Exception:
        continue
    if hasattr(v, "amplitude"):
        print("  %-34s amp=%.3f freq=%.3f offset=%s waveform=%s" % (
            a, v.amplitude, v.frequency,
            v.get_editor_property("initial_offset"), v.get_editor_property("waveform")))
    else:
        print("  %-34s %s" % (a, v))
