import unreal

BP_PATH = "/Game/Interactables/BP_TreasureChest"
INV_CLASS_PATH = "/Script/NarrativeInventory.NarrativeInventoryComponent"

def mark(k, *v):
    unreal.log("CHESTMARK|%s|%s" % (k, " ".join(str(x) for x in v)))

bp = unreal.load_asset(BP_PATH)
mark("BP", bp)

inv_cls = getattr(unreal, "NarrativeInventoryComponent", None)
if inv_cls is None:
    inv_cls = unreal.load_class(None, INV_CLASS_PATH)
mark("INVCLASS", inv_cls)

sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sds.k2_gather_subobject_data_for_blueprint(bp)
mark("HANDLES", len(handles))

# Find existing NarrativeInventoryComponent (guard against 016 partial add), else add new
comp = None
for h in handles:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if obj and isinstance(obj, unreal.NarrativeInventoryComponent):
        comp = obj
        mark("FOUND_EXISTING", obj.get_name())
        break

if comp is None:
    root = handles[0]
    params = unreal.AddNewSubobjectParams(parent_handle=root, new_class=inv_cls, blueprint_context=bp)
    new_h, fail = sds.add_new_subobject(params)
    mark("ADD_FAIL_REASON", fail.to_string() if fail else "none")
    data = sds.k2_find_subobject_data_from_handle(new_h)
    comp = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    mark("ADDED", comp.get_name() if comp else "None")

# Set properties
comp.set_editor_property("InventoryFriendlyName", "Chest")
comp.set_editor_property("Capacity", 10)
mark("SET_NAME", comp.get_editor_property("InventoryFriendlyName"))
mark("SET_CAP", comp.get_editor_property("Capacity"))

# Compile + save
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
mark("SAVED", saved)
mark("DONE", "ok")
