
import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
unreal.SystemLibrary.execute_console_command(world, 'Shot ShowUI')
print('NEXUS_SHOT_REQ')
