import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc = unreal.GameplayStatics.get_player_controller(world, 0)
print("[HUDTEST] pc =", pc)

comp = pc.get_component_by_class(unreal.load_class(None, "/Script/Nexus.NexusInventoryUIComponent"))
print("[HUDTEST] comp =", comp)

# put some items in the run inventory so the menu has content
ps = pc.get_editor_property("PlayerState")
inv = None
for c in ps.get_components_by_class(unreal.load_class(None, "/Script/NarrativeInventory.NarrativeInventoryComponent")):
    print("[HUDTEST] inv comp:", c.get_name())
    if c.get_name() == "RunInventory":
        inv = c
print("[HUDTEST] run inventory =", inv)
for path, qty in [("/Game/Inventory/Items/BP_Item_Gold.BP_Item_Gold_C", 137),
                  ("/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C", 3),
                  ("/Game/Inventory/Items/BP_Item_Weapon_RustedAxe.BP_Item_Weapon_RustedAxe_C", 1)]:
    cls = unreal.load_class(None, path)
    r = inv.try_add_item_from_class(cls, qty)
    print("[HUDTEST] add", path.split(".")[-1], "given=", r.get_editor_property("amount_given"))

comp.call_method("ToggleInventoryMenu")
print("[HUDTEST] toggle called")
