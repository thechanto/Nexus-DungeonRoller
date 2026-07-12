import unreal
import re

AL = unreal.AnimationLibrary
PATHS = [
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
]

for p in PATHS:
    m = unreal.load_asset(p)
    print("=== %s  len=%.3f" % (m.get_name(), AL.get_sequence_length(m)))
    for e in AL.get_animation_notify_events(m):
        n = e.get_editor_property("notify")
        cls = n.get_class().get_name() if n else "?"
        txt = e.export_text()
        # pull the linkable-element time fields out of the struct text
        fields = {}
        for key in ["LinkValue", "SegmentBeginTime", "SegmentLength", "LinkMethod", "SegmentIndex"]:
            mm = re.search(key + r"=([^,\)]+)", txt)
            if mm:
                fields[key] = mm.group(1)
        print("    %-22s %s" % (cls, fields))
    print("")
