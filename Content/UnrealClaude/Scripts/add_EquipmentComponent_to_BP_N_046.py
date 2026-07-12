import unreal

bp_path = "/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer"
comp_class = unreal.load_class(None, "/Script/NarrativeEquipment.EquipmentComponent")
print("EQUIP_CLASS:", comp_class)

bp = unreal.load_asset(bp_path)
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sds.k2_gather_subobject_data_for_blueprint(bp)

# Enumerate existing components + detect an existing equipment comp (idempotent)
existing = []
have_equip = False
for h in handles:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if obj:
        cn = obj.get_class().get_name()
        existing.append(obj.get_name() + " : " + cn)
        if "EquipmentComponent" in cn:
            have_equip = True
print("EXISTING_COMPONENTS:", existing)
print("HAVE_EQUIP_ALREADY:", have_equip)

root = handles[0]
# find the mesh root to parent under (optional); parent under root is fine
if not have_equip:
    params = unreal.AddNewSubobjectParams(parent_handle=root, new_class=comp_class, blueprint_context=bp)
    new_handle, fail = sds.add_new_subobject(params)
    print("ADD_FAIL_REASON:", fail)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    saved = unreal.EditorAssetLibrary.save_asset(bp_path)
    print("ADDED_EQUIP: True  SAVED:", saved)
else:
    print("ADDED_EQUIP: False (already present)")

# Re-read to confirm
handles2 = sds.k2_gather_subobject_data_for_blueprint(bp)
final = []
for h in handles2:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if obj:
        final.append(obj.get_class().get_name())
print("FINAL_COMPONENT_CLASSES:", final)
