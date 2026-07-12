import unreal

out = unreal.Paths.project_dir() + "_ai/t3d"
at = unreal.AssetToolsHelpers.get_asset_tools()

am = unreal.load_object(None, "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher.AM_SkyCrusher")
unreal.log("MONTAGE len=%s rate=%s" % (
    am.get_editor_property("sequence_length"), am.get_editor_property("rate_scale")))

for t in unreal.AnimationLibrary.get_animation_notify_track_names(am):
    unreal.log("TRACK %s" % t)
for ev in unreal.AnimationLibrary.get_animation_notify_events(am):
    try:
        n = ev.get_editor_property("notify")
        unreal.log("NOTIFY t=%.3f cls=%s" % (
            ev.get_editor_property("trigger_time_offset") or 0.0,
            type(n).__name__ if n else "state/none"))
    except Exception as e:
        unreal.log("NOTIFY_ERR %s" % e)

at.export_assets(["/Game/RetargetedAbilities/Warrior/AM_SkyCrusher.AM_SkyCrusher",
                  "/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy.BP_CameraShake_Hit_Enemy"], out)

sh = unreal.load_object(None, "/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy.BP_CameraShake_Hit_Enemy")
unreal.log("SHAKE parent=%s" % sh.get_editor_property("parent_class").get_name())
unreal.log("DONE")
