
import unreal
cls = unreal.load_class(None, '/Script/UnrealEd.LevelEditorPlaySettings')
s = unreal.get_default_object(cls)
props = [p for p in dir(s) if 'mode' in p.lower() or 'window' in p.lower()]
print('NEXUS_PS2|' + str(props))
try:
    print('NEXUS_PS2_CUR|' + str(s.get_editor_property('LastExecutedPlayModeType')))
except Exception as e:
    print('NEXUS_PS2_CUR|ERR ' + str(e))
