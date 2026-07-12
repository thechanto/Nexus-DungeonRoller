
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
ui = pc.get_component_by_class(unreal.NexusInventoryUIComponent)
inv_cls = ui.get_editor_property("InventoryMenuClass")
loot_cls = ui.get_editor_property("LootMenuClass")
print("LMFIX|BEFORE inv", inv_cls, "loot", loot_cls)
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Menus/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
if loot_cls is None:
    ui.set_editor_property("LootMenuClass", menu_cls)
    print("LMFIX|SET_LOOT", ui.get_editor_property("LootMenuClass"))
# find a chest still with items
chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chests = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)
target=None
for ch in chests:
    if len(ch.get_component_by_class(unreal.NarrativeInventoryComponent).get_items())>0: target=ch; break
lib_cdo = unreal.get_default_object(unreal.load_class(None,"/Script/Nexus.NexusAbilityUILibrary"))
ret = lib_cdo.call_method("OpenContainerLoot", (pawn, target))
print("LMFIX|REOPEN_ON", target.get_name(), "RET", ret)
