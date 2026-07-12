import unreal
import re

AL = unreal.AnimationLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()


def dump(a, label):
    print("--- %s : %s (%s) len=%.3f" % (label, a.get_name(), a.get_class().get_name(),
                                         AL.get_sequence_length(a)))
    print("    tracks: %s" % [str(t) for t in AL.get_animation_notify_track_names(a)])
    evs = AL.get_animation_notify_events(a)
    print("    notifies: %d" % len(evs))
    for e in evs:
        n = e.get_editor_property("notify")
        cls = n.get_class().get_name() if n else "?"
        txt = e.export_text()
        f = {}
        for key in ["LinkValue", "LinkMethod", "SegmentIndex", "SlotIndex"]:
            mm = re.search(key + r"=([^,\)]+)", txt)
            if mm:
                f[key] = mm.group(1)
        print("      %-22s %s" % (cls, f))


# the proven working melee montage
axe = None
for a in ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage"), True):
    if "AxeSwing" in str(a.asset_name):
        axe = unreal.load_asset(str(a.package_name))
        break

if not axe:
    print("axe montage not found")
else:
    dump(axe, "AXE MONTAGE")
    # its source sequence
    deps = unreal.EditorAssetLibrary.find_package_referencers_for_asset  # noqa
    src = None
    for d in unreal.AssetRegistryHelpers.get_asset_registry().get_dependencies(
            unreal.TopLevelAssetPath if False else axe.get_outer().get_name(),
            unreal.AssetRegistryDependencyOptions()):
        pass
    # simpler: pull the sequence off the montage's slot track via T3D of the montage
    t3d = axe.export_text()
    m = re.search(r"AnimReference=[\"']?AnimSequence'\"?([^'\"]+)", t3d)
    print("    slot anim ref regex hit: %s" % (m.group(1) if m else None))
    for mm in re.finditer(r"AnimSequence'([^']+)'", t3d):
        print("    -> segment sequence: %s" % mm.group(1))
        s = unreal.load_asset(mm.group(1).split(".")[0] if "." in mm.group(1) else mm.group(1))
        if s:
            dump(s, "AXE SOURCE SEQ")
        break
