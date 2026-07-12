import unreal

AL = unreal.AnimationLibrary
PATHS = [
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
]

first = True
for p in PATHS:
    m = unreal.load_asset(p)
    print("=== %s  len=%.3f" % (m.get_name(), AL.get_sequence_length(m)))
    evs = AL.get_animation_notify_events(m)
    print("    notify count = %d" % len(evs))

    if first and evs:
        e0 = evs[0]
        props = [a for a in dir(e0) if not a.startswith("_")]
        print("    AnimNotifyEvent attrs: %s" % props)
        first = False

    for e in evs:
        cls = "None"
        try:
            n = e.get_editor_property("notify")
            if n:
                cls = n.get_class().get_name()
        except Exception as ex:
            cls = "err:%s" % ex

        t = None
        for cand in ["trigger_time", "display_time", "link_value", "time"]:
            try:
                t = e.get_editor_property(cand)
                tname = cand
                break
            except Exception:
                continue
        print("    %-26s  %s=%s" % (cls, tname if t is not None else "time", t))
    print("")
