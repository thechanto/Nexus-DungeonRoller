import unreal

MARK = "LOOT17"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = unreal.GameplayStatics.get_player_pawn(world, 0)
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)

gold = lib.call_method("GetGold", (player,))
if gold != 0:
    lib.call_method("AddGold", (player, -gold))
unreal.log(MARK + ": gold reset %s -> %s" % (gold, lib.call_method("GetGold", (player,))))

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_end_play()
unreal.log(MARK + ": PIE end requested")
