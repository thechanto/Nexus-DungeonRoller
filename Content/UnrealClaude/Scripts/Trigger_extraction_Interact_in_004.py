import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = ues.get_game_world()
pc = unreal.GameplayStatics.get_player_controller(w, 0)
pawn = pc.get_editor_property("pawn")
unreal.log_warning("NEXUS_CHECK pawn=%s" % (pawn.get_name() if pawn else "NONE"))
cls = unreal.load_class(None, "/Game/Test/BP_InteractionTest.BP_InteractionTest_C")
actors = unreal.GameplayStatics.get_all_actors_of_class(w, cls)
if actors:
    try:
        actors[0].call_method("Interact", (pawn,))
        unreal.log_warning("NEXUS_CHECK Interact(pawn) called")
    except Exception as e:
        actors[0].call_method("Interact")
        unreal.log_warning("NEXUS_CHECK Interact() called no-arg (arg call failed: %s)" % e)
else:
    unreal.log_error("NEXUS_CHECK no interaction actor")
