import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = unreal.GameplayStatics.get_player_character(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
comp = player.get_component_by_class(unreal.load_class(None, "/Script/Nexus.NexusLockOnComponent"))

target = comp.call_method("GetLockedTarget")
unreal.log("LOCKON09b: still_locked_target=" + str(target))
if target:
    wcs = target.get_components_by_class(unreal.WidgetComponent)
    for w in wcs:
        unreal.log("LOCKON09b: LOCK indicator widget_class=%s attach_socket=%s" % (
            str(w.get_editor_property("widget_class")), str(w.get_attach_socket_name())))

# toggle off and verify full restore
comp.call_method("ToggleLockOn")
t2 = comp.call_method("GetLockedTarget")
mv = player.get_editor_property("character_movement")
unreal.log("LOCKON09b: UNLOCK target=%s orient=%s desired=%s ignore_look=%s" % (
    str(t2),
    mv.get_editor_property("orient_rotation_to_movement"),
    mv.get_editor_property("use_controller_desired_rotation"),
    pc.is_look_input_ignored()))
if target:
    remaining = [w for w in target.get_components_by_class(unreal.WidgetComponent)
                 if "LockOn" in str(w.get_editor_property("widget_class"))]
    unreal.log("LOCKON09b: UNLOCK lockon_widgets_remaining=%d" % len(remaining))
unreal.log("LOCKON09b: DONE")
