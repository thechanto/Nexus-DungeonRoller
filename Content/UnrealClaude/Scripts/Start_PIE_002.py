import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_begin_play()
unreal.log_warning("NEXUS_PIE begin_play requested")
