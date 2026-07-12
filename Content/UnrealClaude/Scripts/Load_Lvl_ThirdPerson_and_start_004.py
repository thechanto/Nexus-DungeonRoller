import unreal
unreal.EditorLoadingAndSavingUtils.load_map("/Game/ThirdPerson/Lvl_ThirdPerson")
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_begin_play()
unreal.log("PIE_REQUESTED")
