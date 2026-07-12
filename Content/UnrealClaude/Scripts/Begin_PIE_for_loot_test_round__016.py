import unreal

MARK = "LOOT12"
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_begin_play()
unreal.log(MARK + ": PIE begin requested")
