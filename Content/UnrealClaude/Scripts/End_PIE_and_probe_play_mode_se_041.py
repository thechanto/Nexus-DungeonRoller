
import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.editor_request_end_play()
s = unreal.get_default_object(unreal.LevelEditorPlaySettings)
props = [p for p in dir(s) if 'play' in p.lower() or 'window' in p.lower() or 'mode' in p.lower()]
print('NEXUS_PS|' + str(props))
try:
    cur = s.get_editor_property('last_executed_play_mode_type')
    print('NEXUS_PS_CUR|' + str(cur))
except Exception as e:
    print('NEXUS_PS_CUR|ERR ' + str(e))
