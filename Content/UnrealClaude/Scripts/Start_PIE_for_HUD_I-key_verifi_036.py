import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_begin_play()
print("NEXUS_PIE_REQUESTED")