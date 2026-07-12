import unreal, os

L = []
def p(s):
    L.append(str(s))
    unreal.log("RM2| " + str(s))

# ---- ABP root motion mode via the real player BP path ----------------------
bpc = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer")
p("PLAYER_CLASS=%s" % bpc)
cdo = unreal.get_default_object(bpc)
mesh = cdo.get_editor_property("mesh")
anim_class = mesh.get_editor_property("anim_class")
p("mesh.anim_class = %s" % anim_class)
p("mesh.skeletal_mesh_asset = %s" % mesh.get_editor_property("skeletal_mesh_asset"))
if anim_class:
    acdo = unreal.get_default_object(anim_class)
    try:
        p("*** ABP.root_motion_mode = %s ***" % acdo.get_editor_property("root_motion_mode"))
    except Exception as e:
        p("ABP.root_motion_mode ERR %s" % e)

# ---- finer root-Z samples around launch and landing ------------------------
seq = unreal.EditorAssetLibrary.load_asset("/Game/RetargetedAbilities/Warrior/Ability_Ultimate")
p("FINE SAMPLES root.Z:")
t = 1.30
while t <= 1.65:
    xf = unreal.AnimationLibrary.get_bone_pose_for_time(seq, "root", t, False)
    p("  t=%5.3f Z=%9.2f" % (t, xf.translation.z))
    t += 0.025
p("  ---- descent/landing ----")
t = 3.15
while t <= 3.45:
    xf = unreal.AnimationLibrary.get_bone_pose_for_time(seq, "root", t, False)
    p("  t=%5.3f Z=%9.2f" % (t, xf.translation.z))
    t += 0.025

# ---- FRESH montage T3D export (authoritative notify read) ------------------
out_dir = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/T3D_Verify"
os.makedirs(out_dir, exist_ok=True)
mont = unreal.EditorAssetLibrary.load_asset("/Game/RetargetedAbilities/Warrior/AM_SkyCrusher")
task = unreal.AssetExportTask()
task.set_editor_property("automated", True)
task.set_editor_property("object", mont)
task.set_editor_property("filename", out_dir + "/AM_SkyCrusher_fresh.T3D")
task.set_editor_property("prompt", False)
task.set_editor_property("replace_identical", True)
task.set_editor_property("write_empty_files", False)
ok = unreal.Exporter.run_asset_export_task(task)
p("montage T3D export ok=%s" % ok)

# ---- also list notify events via the API (times unreliable, names OK) ------
try:
    tracks = unreal.AnimationLibrary.get_animation_notify_track_names(mont)
    p("notify tracks = %s" % [str(t_) for t_ in tracks])
    evs = unreal.AnimationLibrary.get_animation_notify_events(mont)
    p("notify event count = %d" % len(evs))
    for e in evs:
        nfy = e.get_editor_property("notify")
        st = e.get_editor_property("notify_state_class")
        p("  notify=%s  state=%s" % (nfy, st))
except Exception as e:
    p("notify api ERR %s" % e)

with open("C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/b1b_out.txt", "w") as f:
    f.write("\n".join(L))
p("DONE")
