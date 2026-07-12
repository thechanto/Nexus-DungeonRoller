import unreal

BP_PATH = "/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer"

sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
bp = unreal.load_asset(BP_PATH)
if not bp:
    raise Exception("Could not load " + BP_PATH)

handles = sds.k2_gather_subobject_data_for_blueprint(bp)
print("=== Existing components on BP_NexusPlayer ===")
root = None
existing = []
for h in handles:
    d = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
    if obj:
        nm = obj.get_name()
        existing.append(nm)
        print("   ", nm, "|", obj.get_class().get_name())
    if unreal.SubobjectDataBlueprintFunctionLibrary.is_root_actor(d):
        root = h

already = any("NarrativeInventory" in e for e in existing)
print("")
if already:
    print("RESULT: NarrativeInventoryComponent ALREADY present - no change made.")
else:
    params = unreal.AddNewSubobjectParams(
        parent_handle=root,
        new_class=unreal.NarrativeInventoryComponent,
        blueprint_context=bp,
    )
    new_handle, fail = sds.add_new_subobject(params)
    if fail and not fail.is_empty():
        print("RESULT: ADD FAILED ->", fail)
    else:
        sds.rename_subobject(handle=new_handle, new_name="InventoryComponent")
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
        print("RESULT: Added NarrativeInventoryComponent as 'InventoryComponent', compiled + saved.")

print("")
print("=== Verify after ===")
for h in sds.k2_gather_subobject_data_for_blueprint(bp):
    d = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
    if obj and "Inventory" in obj.get_name():
        print("   FOUND:", obj.get_name(), "|", obj.get_class().get_path_name())
