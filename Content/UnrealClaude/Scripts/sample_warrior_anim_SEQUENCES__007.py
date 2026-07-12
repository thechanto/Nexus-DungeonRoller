import unreal

SEQS = [
    ("AM_SkyCrusher", "/Game/RetargetedAbilities/Warrior/Ability_Ultimate"),
    ("AM_SwordSweep", "/Game/RetargetedAbilities/Warrior/Ability_E"),
    ("AM_ShieldSlam", "/Game/RetargetedAbilities/Warrior/Ability_Q"),
    ("AM_LeapSlash", "/Game/RetargetedAbilities/Warrior/Ability_R"),
]

BONES = ["hand_r", "root"]

for label, path in SEQS:
    s = unreal.load_asset(path)
    if not s:
        print("MISSING %s" % path)
        continue
    length = unreal.AnimationLibrary.get_sequence_length(s)
    names = [str(b) for b in unreal.AnimationLibrary.get_animation_track_names(s)]
    print("=== %s  (%s)  len=%.3f  tracks=%d" % (label, s.get_name(), length, len(names)))

    for bone in BONES:
        if bone not in names:
            print("    -- bone %s not in tracks; sample: %s" % (bone, names[:8]))
            continue
        N = 70
        prev = None
        rows = []
        for i in range(N + 1):
            t = length * i / float(N)
            tf = unreal.AnimationLibrary.get_bone_pose_for_time(s, bone, t, True)
            loc = tf.translation
            spd = 0.0
            if prev is not None:
                dx, dy, dz = loc.x - prev.x, loc.y - prev.y, loc.z - prev.z
                dt = length / float(N)
                spd = ((dx * dx + dy * dy + dz * dz) ** 0.5) / max(dt, 1e-6)
            rows.append((t, loc.z, spd))
            prev = loc

        peak = max(rows, key=lambda r: r[2])
        # impact = biggest single-frame DECELERATION after the speed peak
        best_dec, best_t = 0.0, peak[0]
        for i in range(1, len(rows)):
            if rows[i][0] <= peak[0]:
                continue
            dec = rows[i - 1][2] - rows[i][2]
            if dec > best_dec:
                best_dec, best_t = dec, rows[i][0]

        print("  [%s] PEAK v=%.0f @ t=%.3f   | max decel %.0f @ t=%.3f  <-- impact est" % (
            bone, peak[2], peak[0], best_dec, best_t))
        for (t, z, sp) in rows:
            mark = ""
            if abs(t - best_t) < 1e-6:
                mark = "   <<< IMPACT"
            elif abs(t - peak[0]) < 1e-6:
                mark = "   <<< peak speed"
            bar = "#" * int(min(sp / 120.0, 36))
            print("    t=%.3f z=%8.1f v=%7.0f %s%s" % (t, z, sp, bar, mark))
    print("")
