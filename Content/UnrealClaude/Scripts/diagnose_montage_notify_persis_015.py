import unreal

AL = unreal.AnimationLibrary
P = "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher"

m = unreal.load_asset(P)
START_CLS = unreal.load_class(None, "/Game/AnimNotifies/AN_HitScanStart.AN_HitScanStart_C")

print("BEFORE: tracks=%s notifies=%d" % (
    [str(t) for t in AL.get_animation_notify_track_names(m)],
    len(AL.get_animation_notify_events(m))))

# Is the Notifies array readable/writable via reflection?
try:
    n = m.get_editor_property("notifies")
    print("get notifies OK: type=%s len=%s" % (type(n), len(n)))
except Exception as e:
    print("get notifies FAILED: %s" % e)

# Use the montage's existing track "1" (matches the working axe montage) instead of a new track
tracks = [str(t) for t in AL.get_animation_notify_track_names(m)]
TRACK = "1" if "1" in tracks else tracks[0]
print("using track '%s'" % TRACK)

ev = AL.add_animation_notify_event(m, TRACK, 3.25, START_CLS)
print("add returned: %s" % ev)
print("AFTER ADD: notifies=%d" % len(AL.get_animation_notify_events(m)))

# force the montage to rebuild its cached notify/track data + mark dirty
try:
    m.modify()
except Exception as e:
    print("modify: %s" % e)
try:
    m.post_edit_change()
    print("post_edit_change ok")
except Exception as e:
    print("post_edit_change: %s" % e)

pkg = m.get_outer()
try:
    dirty = pkg.is_dirty()
except Exception:
    dirty = "?"
print("package dirty before save: %s" % dirty)

saved = unreal.EditorAssetLibrary.save_asset(P, only_if_is_dirty=False)
print("saved=%s" % saved)
print("AFTER SAVE (in-memory): notifies=%d" % len(AL.get_animation_notify_events(m)))

# export T3D right here and print the Notifies lines from the freshly written file
unreal.AssetTools.export_assets([m], "C:/Temp/nexus_t3d")
print("--- exported")
