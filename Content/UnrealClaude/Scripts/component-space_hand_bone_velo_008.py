import unreal

SEQS = [
    ("AM_SkyCrusher", "/Game/RetargetedAbilities/Warrior/Ability_Ultimate"),
    ("AM_SwordSweep", "/Game/RetargetedAbilities/Warrior/Ability_E"),
    ("AM_ShieldSlam", "/Game/RetargetedAbilities/Warrior/Ability_Q"),
    ("AM_LeapSlash", "/Game/RetargetedAbilities/Warrior/Ability_R"),
]

ML = unreal.MathLibrary
AL = unreal.AnimationLibrary


def comp_space(seq, chain, t):
    """chain = [bone, parent, ..., root]; compose local transforms up to root."""
    acc = unreal.Transform()
    for bone in reversed(chain):          # root first
        loc = AL.get_bone_pose_for_time(seq, bone, t, False)
        acc = ML.compose_transforms(loc, acc)   # local * parentComponent
    return acc.translation


for label, path in SEQS:
    seq = unreal.load_asset(path)
    length = AL.get_sequence_length(seq)
    print("=== %s (%s) len=%.3f" % (label, seq.get_name(), length))

    for bone in ["hand_r", "hand_l"]:
        try:
            chain = [str(b) for b in AL.find_bone_path_to_root(seq, bone)]
        except Exception as e:
            print("  chain fail %s: %s" % (bone, e))
            continue
        if not chain:
            print("  no chain for %s" % bone)
            continue

        N = 70
        prev = None
        rows = []
        for i in range(N + 1):
            t = length * i / float(N)
            p = comp_space(seq, chain, t)
            spd = 0.0
            if prev is not None:
                dx, dy, dz = p.x - prev.x, p.y - prev.y, p.z - prev.z
                dt = length / float(N)
                spd = ((dx * dx + dy * dy + dz * dz) ** 0.5) / max(dt, 1e-6)
            rows.append((t, p.z, spd))
            prev = p

        peak = max(rows, key=lambda r: r[2])
        best_dec, best_t = 0.0, peak[0]
        for i in range(1, len(rows)):
            if rows[i][0] <= peak[0]:
                continue
            dec = rows[i - 1][2] - rows[i][2]
            if dec > best_dec:
                best_dec, best_t = dec, rows[i][0]

        print("  [%s] chain=%d  PEAK v=%.0f @ t=%.3f | max decel %.0f @ t=%.3f  <-- IMPACT est" % (
            len(chain) and bone or bone, len(chain), peak[2], peak[0], best_dec, best_t))
        for (t, z, sp) in rows:
            mark = ""
            if abs(t - best_t) < 1e-6:
                mark = "  <<< IMPACT"
            elif abs(t - peak[0]) < 1e-6:
                mark = "  <<< peak v"
            bar = "#" * int(min(sp / 40.0, 34))
            print("    t=%.3f z=%7.1f v=%6.0f %s%s" % (t, z, sp, bar, mark))
    print("")
