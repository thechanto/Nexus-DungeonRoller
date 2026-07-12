import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Character):
    if "NexusPlayer" in p.get_name():
        player = p
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
basic = asc.get_attribute_set(unreal.BasicAttributeSet)
if not basic:
    raise RuntimeError("no BasicAttributeSet")
out = []
for prop in ["max_health", "health", "max_stamina", "stamina", "max_mana", "mana"]:
    d = basic.get_editor_property(prop)
    out.append(f"{prop}={d.get_editor_property('current_value'):.1f}")
print("[PIE-CHECK] " + ", ".join(out))
