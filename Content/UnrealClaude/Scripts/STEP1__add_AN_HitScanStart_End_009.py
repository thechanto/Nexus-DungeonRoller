import unreal

AL = unreal.AnimationLibrary
TRACK = "HitScan"

START_CLS = unreal.load_class(None, "/Game/AnimNotifies/AN_HitScanStart.AN_HitScanStart_C")
END_CLS = unreal.load_class(None, "/Game/AnimNotifies/AN_HitScanEnd.AN_HitScanEnd_C")
print("start_cls=%s end_cls=%s" % (START_CLS, END_CLS))

# montage -> (hitscan_start_time, hitscan_end_time)  [derived from bone sampling]
PLAN = {
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher": (3.25, 3.45),   # slam: root lands t=3.28
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep": (0.50, 0.75),   # contact 0.615
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam": (0.20, 0.45),   # contact 0.347
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash": (0.85, 1.10),    # contact 0.933
}

for path, (t_start, t_end) in PLAN.items():
    m = unreal.load_asset(path)
    if not m:
        print("MISSING %s" % path)
        continue
    name = m.get_name()
    length = AL.get_sequence_length(m)

    tracks = [str(t) for t in AL.get_animation_notify_track_names(m)]
    if TRACK not in tracks:
        AL.add_animation_notify_track(m, TRACK, unreal.LinearColor(1, 1, 1, 1))
        print("%s: added track '%s' (had %s)" % (name, TRACK, tracks))

    # idempotent: drop any existing HitScan notifies before re-adding
    for cls_name in ["AN_HitScanStart_C", "AN_HitScanEnd_C"]:
        try:
            removed = AL.remove_animation_notify_events_by_name(m, cls_name)
        except Exception:
            removed = 0

    ev = AL.get_animation_notify_events(m)
    print("%s: len=%.3f  existing notifies=%d  tracks=%s" % (
        name, length, len(ev), [str(t) for t in AL.get_animation_notify_track_names(m)]))

    ok1 = AL.add_animation_notify_event(m, TRACK, t_start, START_CLS)
    ok2 = AL.add_animation_notify_event(m, TRACK, t_end, END_CLS)

    ev2 = AL.get_animation_notify_events(m)
    got = []
    for e in ev2:
        try:
            n = e.get_editor_property("notify")
            trig = e.get_editor_property("trigger_time_offset")
            got.append("%s@%.3f" % (n.get_class().get_name() if n else "?", e.get_editor_property("link_value")))
        except Exception as ex:
            got.append("?(%s)" % ex)

    saved = unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
    print("  -> added start@%.2f end@%.2f | notifies now=%d %s | saved=%s" % (
        t_start, t_end, len(ev2), got, saved))
    print("")

print("DONE")
