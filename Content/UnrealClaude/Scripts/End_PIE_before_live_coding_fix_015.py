import unreal

MARK = "LOOT14"
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_end_play()
unreal.log(MARK + ": PIE end requested")
