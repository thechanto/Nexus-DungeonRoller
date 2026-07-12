import unreal
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_end_play()
unreal.log_warning("NEXUS_PIE end_play requested")
