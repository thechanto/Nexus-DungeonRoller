import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
try:
    les.editor_request_end_play()
    unreal.log("PIE_ENDED")
except Exception as e:
    unreal.log_error("end pie: %s" % e)
