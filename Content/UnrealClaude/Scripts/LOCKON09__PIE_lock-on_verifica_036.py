import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = unreal.GameplayStatics.get_player_character(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
unreal.log("LOCKON09: world=%s player=%s" % (world.get_name() if world else None, player.get_name() if player else None))

comp_cls = unreal.load_class(None, "/Script/Nexus.NexusLockOnComponent")
comp = player.get_component_by_class(comp_cls)
unreal.log("LOCKON09: comp=" + str(comp))

# resolved asset fallbacks (BeginPlay should have filled these)
lib = unreal.get_default_object(unreal.load_class(None, "/Script/Nexus.NexusAbilityUILibrary"))
enemy_cls = unreal.load_class(None, "/Script/Nexus.NexusEnemyBase")
enemies = unreal.GameplayStatics.get_all_actors_of_class(world, enemy_cls)
unreal.log("LOCKON09: enemies=%d" % len(enemies))

ploc = player.get_actor_location()
nearest = min(enemies, key=lambda e: (e.get_actor_location() - ploc).length())
dist = (nearest.get_actor_location() - ploc).length()
unreal.log("LOCKON09: nearest=%s dist=%.0f" % (nearest.get_name(), dist))
if dist > 1500:
    dest = nearest.get_actor_location() + unreal.Vector(400.0, 0.0, 100.0)
    player.set_actor_location(dest, False, True)
    unreal.log("LOCKON09: teleported next to enemy")
look = unreal.MathLibrary.find_look_at_rotation(player.get_actor_location(), nearest.get_actor_location())
pc.set_control_rotation(look)

# --- lock ---
comp.call_method("ToggleLockOn")
target = comp.call_method("GetLockedTarget")
unreal.log("LOCKON09: LOCK target=" + str(target))
mv = player.get_editor_property("character_movement")
unreal.log("LOCKON09: LOCK orient=%s desired=%s ignore_look=%s" % (
    mv.get_editor_property("orient_rotation_to_movement"),
    mv.get_editor_property("use_controller_desired_rotation"),
    pc.is_look_input_ignored()))
if target:
    wcs = target.get_components_by_class(unreal.WidgetComponent)
    for w in wcs:
        unreal.log("LOCKON09: LOCK indicator widget_class=%s space=%s" % (
            str(w.get_editor_property("widget_class")), str(w.get_editor_property("widget_space"))))

# --- toggle off ---
comp.call_method("ToggleLockOn")
t2 = comp.call_method("GetLockedTarget")
unreal.log("LOCKON09: UNLOCK target=%s orient=%s desired=%s ignore_look=%s" % (
    str(t2),
    mv.get_editor_property("orient_rotation_to_movement"),
    mv.get_editor_property("use_controller_desired_rotation"),
    pc.is_look_input_ignored()))
wcs2 = nearest.get_components_by_class(unreal.WidgetComponent)
lockon_widgets = [w for w in wcs2 if "LockOn" in str(w.get_editor_property("widget_class"))]
unreal.log("LOCKON09: UNLOCK lockon_widgets_remaining=%d" % len(lockon_widgets))
unreal.log("LOCKON09: DONE")
