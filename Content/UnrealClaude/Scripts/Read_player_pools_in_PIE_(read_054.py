import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
print('WORLD=' + str(world))
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
print('PAWN=' + str(pawn))
asc = pawn.get_components_by_class(unreal.AbilitySystemComponent)[0]
bas = asc.get_attribute_set(unreal.BasicAttributeSet)
for a in ['Health', 'MaxHealth', 'Stamina', 'MaxStamina', 'Mana', 'MaxMana']:
    print(a + '=' + bas.get_editor_property(a).export_text())
