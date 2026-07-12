import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_end_play()
print("PIEVERIFY end_play requested")
