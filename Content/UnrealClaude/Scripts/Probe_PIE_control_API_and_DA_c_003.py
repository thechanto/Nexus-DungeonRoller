import unreal
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.log("LES_PLAY: " + str([m for m in dir(les) if 'play' in m.lower() or 'pie' in m.lower()]))
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
unreal.log("UES: " + str([m for m in dir(ues) if 'world' in m.lower()]))
unreal.log("ELL_PLAY: " + str([m for m in dir(unreal.EditorLevelLibrary) if 'play' in m.lower() or 'game' in m.lower()]))
for p in ["/Game/GameplayAbilitySystem/AbilityData/DA_Ability_ShieldSlam.DA_Ability_ShieldSlam_C"]:
    c = unreal.load_class(None, p)
    unreal.log("LOADCLASS %s -> %s" % (p, c))
unreal.log("PROBE_DONE")
