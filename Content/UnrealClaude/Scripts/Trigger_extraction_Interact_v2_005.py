import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = ues.get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
unreal.log_warning("NEXUS_CHECK2 pawn=%s" % (pawn.get_name() if pawn else "NONE"))
cls = unreal.load_class(None, "/Game/Test/BP_InteractionTest.BP_InteractionTest_C")
actors = unreal.GameplayStatics.get_all_actors_of_class(w, cls)
unreal.log_warning("NEXUS_CHECK2 interaction actors=%d" % len(actors))
if actors:
    try:
        actors[0].call_method("Interact", (pawn,))
        unreal.log_warning("NEXUS_CHECK2 Interact(pawn) called")
    except Exception as e:
        actors[0].call_method("Interact")
        unreal.log_warning("NEXUS_CHECK2 Interact() called no-arg (arg call failed: %s)" % e)
