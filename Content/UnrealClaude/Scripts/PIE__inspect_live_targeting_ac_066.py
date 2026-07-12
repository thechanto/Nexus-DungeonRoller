import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.NexusCharacterBase):
    if "BP_NexusPlayer" in p.get_class().get_name():
        player = p
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
# find live targeting actors
tas = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.GameplayAbilityTargetActor)
print("TARGET ACTORS: %d" % len(tas))
for ta in tas:
    print("  class=%s" % ta.get_class().get_name())
    try:
        print("  confirm_allowed=%s" % ta.call_method("IsConfirmTargetingAllowed"))
    except Exception as e:
        print("  confirm_allowed ERR: %s" % e)
    try:
        print("  should_produce_on_server=%s" % ta.get_editor_property("should_produce_target_data_on_server"))
    except Exception as e:
        print("  sptods ERR: %s" % e)
# try both confirm routes
for m in ["LocalInputConfirm", "TargetConfirm"]:
    try:
        asc.call_method(m)
        print("%s: called OK" % m)
    except Exception as e:
        print("%s ERR: %s" % (m, e))
print("PIE6 DONE")
