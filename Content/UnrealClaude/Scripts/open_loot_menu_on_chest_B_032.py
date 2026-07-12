
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chests = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)
# pick a chest that still has loot
target=None
for ch in chests:
    ci=ch.get_component_by_class(unreal.NarrativeInventoryComponent)
    if len(ci.get_items())>0: target=ch; break
lib_cdo = unreal.get_default_object(unreal.load_class(None,"/Script/Nexus.NexusAbilityUILibrary"))
ret = lib_cdo.call_method("OpenContainerLoot", (pawn, target))
print("MENUOPEN|OPENED_ON", target.get_name(), "RET", ret)
