import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Character):
    if "NexusPlayer" in p.get_name():
        player = p
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
print("[PROBE] asc attr-ish:", [n for n in dir(asc) if "attribute" in n.lower()])
print("[PROBE] module libs:", [n for n in dir(unreal) if "AttributeSet" in n or "AbilitySystemLib" in n or "bilitySystemBlueprint" in n])
