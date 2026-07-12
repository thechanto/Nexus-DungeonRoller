
import unreal
def desc(inv):
    out=[]
    for it in inv.get_items():
        try: q=it.get_editor_property("quantity")
        except Exception: q="?"
        out.append((it.get_class().get_name(), q))
    return out
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
pc = unreal.GameplayStatics.get_player_controller(world, 0)
ui = pc.get_component_by_class(unreal.NexusInventoryUIComponent)
print("FINAL|LOOTMENUCLASS_PERSISTED", ui.get_editor_property("LootMenuClass") is not None)
chest_cls = unreal.load_class(None, "/Game/Interactables/BP_TreasureChest.BP_TreasureChest_C")
chests = unreal.GameplayStatics.get_all_actors_of_class(world, chest_cls)
for ch in chests:
    print("FINAL|CHEST_AUTOLOOT", ch.get_name(), desc(ch.get_component_by_class(unreal.NarrativeInventoryComponent)))
ps = pc.get_editor_property("player_state")
runInv=[c for c in ps.get_components_by_class(unreal.NarrativeInventoryComponent) if "Run" in c.get_name()][0]
print("FINAL|RUNINV_BEFORE", desc(runInv))
chest = chests[0]
lib_cdo = unreal.get_default_object(unreal.load_class(None,"/Script/Nexus.NexusAbilityUILibrary"))
ret = lib_cdo.call_method("OpenContainerLoot", (pawn, chest))
print("FINAL|OPEN_RET", ret, "LOOTSOURCE_SET", runInv.get_editor_property("loot_source") is not None)
