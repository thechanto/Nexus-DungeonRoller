
import unreal
bp_path = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController"
bp = unreal.load_asset(bp_path)
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting_C")
SDS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = SDS.k2_gather_subobject_data_for_blueprint(bp)
found=False
for h in handles:
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(SDS.k2_find_subobject_data_from_handle(h))
    if isinstance(obj, unreal.NexusInventoryUIComponent):
        print("PERSIST|FOUND", obj.get_name(), "loot_before", obj.get_editor_property("LootMenuClass"))
        obj.set_editor_property("LootMenuClass", menu_cls)
        print("PERSIST|SET_TO", obj.get_editor_property("LootMenuClass"))
        found=True
print("PERSIST|FOUND_COMP", found)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
ok = unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
print("PERSIST|SAVED", ok)
# verify template after compile+save
for h in SDS.k2_gather_subobject_data_for_blueprint(bp):
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(SDS.k2_find_subobject_data_from_handle(h))
    if isinstance(obj, unreal.NexusInventoryUIComponent):
        print("PERSIST|VERIFY_AFTER", obj.get_editor_property("LootMenuClass"))
