
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pc = unreal.GameplayStatics.get_player_controller(world, 0)
pawn = pc.get_controlled_pawn()
print("PAWN:", pawn.get_name() if pawn else None)
eq = pawn.get_component_by_class(unreal.load_class(None, "/Script/NarrativeEquipment.EquipmentComponent")) if pawn else None
print("EQUIP_ON_PAWN:", eq.get_name() if eq else None)
# open inventory menu via the controller's InventoryUI component
invui = pc.get_component_by_class(unreal.load_class(None, "/Script/Nexus.NexusInventoryUIComponent"))
print("INVUI:", invui.get_name() if invui else None)
if invui:
    invui.call_method("ToggleInventoryMenu", ())
    print("TOGGLE_CALLED")
