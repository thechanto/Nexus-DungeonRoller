import unreal
def mark(k,v): unreal.log("PROBE|%s|%s"%(k,v))
gs=[m for m in dir(unreal.GameplayStatics) if 'spawn' in m.lower()]
mark("GS_SPAWN", gs)
eas=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
mark("EAS_SPAWN", [m for m in dir(eas) if 'spawn' in m.lower()])
# input injection candidates
mark("HAS_INPUTKEY", hasattr(unreal.PlayerController,'input_key'))
mark("EI_LIB", [m for m in dir(unreal) if 'EnhancedInput' in m])
