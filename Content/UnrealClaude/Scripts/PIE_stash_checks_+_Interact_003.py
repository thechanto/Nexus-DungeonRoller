import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = ues.get_game_world()
unreal.log_warning("NEXUS_CHECK world=%s" % (w.get_name() if w else "NONE"))
pc = unreal.GameplayStatics.get_player_controller(w, 0)
ps = pc.player_state
unreal.log_warning("NEXUS_CHECK ps=%s class=%s" % (ps.get_name(), ps.get_class().get_name()))
for c in ps.get_components_by_class(unreal.ActorComponent):
    unreal.log_warning("NEXUS_CHECK comp %s (%s)" % (c.get_name(), c.get_class().get_name()))
try:
    ex = unreal.NexusAbilityUILibrary.extract_run_inventory_to_stash(ps)
    unreal.log_warning("NEXUS_CHECK extract(empty run) -> %s" % ex)
except Exception as e:
    unreal.log_error("NEXUS_CHECK extract FAILED: %s" % e)
try:
    unreal.NexusAbilityUILibrary.clear_run_inventory(ps)
    unreal.log_warning("NEXUS_CHECK clear_run_inventory OK")
except Exception as e:
    unreal.log_error("NEXUS_CHECK clear FAILED: %s" % e)
try:
    ld = unreal.NexusAbilityUILibrary.load_stash(ps)
    unreal.log_warning("NEXUS_CHECK load_stash -> %s" % ld)
except Exception as e:
    unreal.log_error("NEXUS_CHECK load FAILED: %s" % e)
# end-to-end extraction: trigger Interact on placed BP_InteractionTest
cls = unreal.load_class(None, "/Game/Test/BP_InteractionTest.BP_InteractionTest_C")
actors = unreal.GameplayStatics.get_all_actors_of_class(w, cls)
unreal.log_warning("NEXUS_CHECK interaction actors=%d" % len(actors))
if actors:
    pawn = pc.k2_get_pawn()
    try:
        actors[0].call_method("Interact", (pawn,))
        unreal.log_warning("NEXUS_CHECK Interact(pawn) called")
    except Exception as e:
        try:
            actors[0].call_method("Interact")
            unreal.log_warning("NEXUS_CHECK Interact() called (no-arg)")
        except Exception as e2:
            unreal.log_error("NEXUS_CHECK Interact FAILED: %s / %s" % (e, e2))
