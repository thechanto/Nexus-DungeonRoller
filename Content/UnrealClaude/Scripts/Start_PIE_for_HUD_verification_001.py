import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
try:
    les.editor_request_begin_play()
    unreal.log("PIE_REQUESTED")
except Exception as e:
    unreal.log_error("PIE start failed: %s" % e)
