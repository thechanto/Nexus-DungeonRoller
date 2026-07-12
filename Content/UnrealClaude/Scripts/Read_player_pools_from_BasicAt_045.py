import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if not world:
    raise RuntimeError("no PIE world")
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Character):
    if "NexusPlayer" in p.get_name():
        player = p
if not player:
    raise RuntimeError("player pawn not found")
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
if not asc:
    raise RuntimeError("no ASC")
sets = asc.get_editor_property("spawned_attributes")
basic = None
for s in sets:
    if s and "BasicAttributeSet" in s.get_class().get_name():
        basic = s
if not basic:
    raise RuntimeError(f"BasicAttributeSet not found in {[str(s) for s in sets]}")
out = []
for prop in ["max_health", "health", "max_stamina", "stamina", "max_mana", "mana"]:
    d = basic.get_editor_property(prop)
    out.append(f"{prop}={d.get_editor_property('current_value')}")
print("[PIE-CHECK] " + ", ".join(out))
