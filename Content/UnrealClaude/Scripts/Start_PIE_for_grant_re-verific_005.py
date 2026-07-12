import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.is_in_play_in_editor():
    les.editor_request_begin_play()
    unreal.log("GT2 PIE start requested; run grant next tick")
unreal.log("GT2_PRESTART_DONE")
