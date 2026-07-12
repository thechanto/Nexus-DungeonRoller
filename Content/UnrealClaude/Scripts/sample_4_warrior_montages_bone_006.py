import unreal

MONTAGES = [
    "/Game/Characters/Warrior/Animations/AM_SkyCrusher",
    "/Game/Characters/Warrior/Animations/AM_SwordSweep",
    "/Game/Characters/Warrior/Animations/AM_ShieldSlam",
    "/Game/Characters/Warrior/Animations/AM_LeapSlash",
]

BONE_CANDIDATES = ["hand_r", "weapon_r", "lowerarm_r", "RightHand", "root"]


def find_montage(name_frag):
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage"), True)
    out = []
    for a in assets:
        if name_frag.lower() in str(a.asset_name).lower():
            out.append(str(a.package_name))
    return out


for path in MONTAGES:
    m = unreal.load_asset(path)
    if not m:
        found = find_montage(path.split("/")[-1])
        print("MISSING %s -> candidates %s" % (path, found))
        if found:
            m = unreal.load_asset(found[0])
        if not m:
            continue

    length = unreal.AnimationLibrary.get_sequence_length(m)
    print("=== %s  len=%.3f" % (m.get_name(), length))

    names = [str(b) for b in unreal.AnimationLibrary.get_animation_track_names(m)]
    bone = None
    for cand in BONE_CANDIDATES:
        if cand in names:
            bone = cand
            break
    if bone is None:
        bone = names[0] if names else "root"
    print("    bone=%s  (%d tracks)" % (bone, len(names)))

    N = 80
    prev = None
    rows = []
    for i in range(N + 1):
        t = length * i / float(N)
        try:
            tf = unreal.AnimationLibrary.get_bone_pose_for_time(m, bone, t, True)
            loc = tf.translation
        except Exception as e:
            print("    sample fail: %s" % e)
            break
        speed = 0.0
        if prev is not None:
            dx, dy, dz = loc.x - prev.x, loc.y - prev.y, loc.z - prev.z
            dt = length / float(N)
            speed = ((dx * dx + dy * dy + dz * dz) ** 0.5) / max(dt, 1e-6)
        rows.append((t, loc.z, speed))
        prev = loc

    if not rows:
        continue

    peak = max(rows, key=lambda r: r[2])
    print("    PEAK speed %.0f at t=%.3f" % (peak[2], peak[0]))
    for (t, z, s) in rows:
        bar = "#" * int(min(s / 150.0, 40))
        print("    t=%.3f z=%8.1f v=%7.0f %s" % (t, z, s, bar))
    print("")
