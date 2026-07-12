import unreal

AL = unreal.AnimationLibrary
PATHS = [
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
    "/Game/Characters/Warrior/Animations/Montage_MeleeAttack_AxeSwing",
]

for p in PATHS:
    m = unreal.load_asset(p)
    if not m:
        # reference montage may live elsewhere
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        hits = [str(a.package_name) for a in ar.get_assets_by_class(
            unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage"), True)
            if "AxeSwing" in str(a.asset_name)]
        if hits:
            m = unreal.load_asset(hits[0])
    if not m:
        print("MISSING %s" % p)
        continue

    print("=== %s  len=%.3f" % (m.get_name(), AL.get_sequence_length(m)))
    evs = AL.get_animation_notify_events(m)
    print("    notify count = %d" % len(evs))
    for e in evs:
        n = e.get_editor_property("notify")
        ns = e.get_editor_property("notify_state_class")
        cls = "None"
        if n:
            cls = n.get_class().get_name()
        elif ns:
            cls = ns.get_class().get_name() + " (state)"
        t = e.get_editor_property("trigger_time")   # FAnimNotifyEvent::GetTriggerTime()
        track = e.get_editor_property("track_index")
        print("    %-24s  t=%.3f  track=%d" % (cls, t, track))
    print("")
