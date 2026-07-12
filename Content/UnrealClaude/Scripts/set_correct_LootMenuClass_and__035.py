
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
ui = pc.get_component_by_class(unreal.NexusInventoryUIComponent)
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
print("LMFIX2|MENU_CLS", menu_cls)
ui.set_editor_property("LootMenuClass", menu_cls)
print("LMFIX2|SET_LOOT", ui.get_editor_property("LootMenuClass"))
chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chests = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)
target=None
for ch in chests:
    if len(ch.get_component_by_class(unreal.NarrativeInventoryComponent).get_items())>0: target=ch; break
lib_cdo = unreal.get_default_object(unreal.load_class(None,"/Script/Nexus.NexusAbilityUILibrary"))
ret = lib_cdo.call_method("OpenContainerLoot", (pawn, target))
print("LMFIX2|REOPEN_ON", target.get_name() if target else None, "RET", ret)
