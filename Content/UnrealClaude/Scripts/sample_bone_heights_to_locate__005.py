import unreal

seq = unreal.load_object(None, "/Game/RetargetedAbilities/Warrior/Ability_Ultimate.Ability_Ultimate")
unreal.log("SEQ=%s len=%.3f" % (seq.get_name(), seq.get_editor_property("sequence_length")))

bones = ["hand_r", "hand_l", "pelvis", "root"]
L = 4.366667
steps = 45
for b in bones:
    row = []
    for i in range(steps + 1):
        t = L * i / steps
        try:
            xf = unreal.AnimationLibrary.get_bone_pose_for_time(seq, b, t, False)
            row.append("%.2f:%.0f" % (t, xf.translation.z))
        except Exception as e:
            row.append("ERR")
            break
    unreal.log("BONE %s | %s" % (b, " ".join(row)))
unreal.log("DONESLAM")
