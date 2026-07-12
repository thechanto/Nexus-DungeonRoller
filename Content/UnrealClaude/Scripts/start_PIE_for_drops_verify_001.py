import unreal
les=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.is_in_play_in_editor():
    les.editor_request_begin_play()
    unreal.log("PIE_REQUESTED")
else:
    unreal.log("PIE_ALREADY_RUNNING")
