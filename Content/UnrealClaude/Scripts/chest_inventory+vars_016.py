import unreal

DST = "/Game/Interactables/BP_TreasureChest"
eal = unreal.EditorAssetLibrary
bp = eal.load_asset(DST)
print("BP:", bp)

SDS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)

def gather():
    return SDS.k2_gather_subobject_data_for_blueprint(bp)

def obj_of(h):
    d = SDS.k2_find_subobject_data_from_handle(h)
    return unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)

handles = gather()
root = handles[0]

seen = {}
for h in handles:
    o = obj_of(h)
    if o and o.get_name() not in seen:
        seen[o.get_name()] = o
print("--- components ---")
for nm,o in seen.items():
    print("  ", nm, o.get_class().get_name())

has_inv = any(o.get_class()==unreal.NarrativeInventoryComponent for o in seen.values())
print("has NarrativeInventoryComponent:", has_inv)

if not has_inv:
    params = unreal.AddNewSubobjectParams(parent_handle=root, new_class=unreal.NarrativeInventoryComponent, blueprint_context=bp)
    new_h, fail = SDS.add_new_subobject(params)
    if not fail.is_empty():
        print("add inv FAIL:", fail)
    else:
        SDS.rename_subobject(new_h, unreal.Text.from_string("ChestInventory"))
        print("added ChestInventory")

# refresh + find inventory template
handles = gather()
inv = None
smc = None
for h in handles:
    o = obj_of(h)
    if not o: continue
    if o.get_class()==unreal.NarrativeInventoryComponent: inv = o
    if o.get_class()==unreal.StaticMeshComponent: smc = o

if inv:
    try: inv.set_editor_property("inventory_friendly_name", unreal.Text.from_string("Chest"))
    except Exception as e: print("friendly err", e)
    inv.set_editor_property("capacity", 10)
    try: inv.set_editor_property("weight_capacity", 1000.0)
    except Exception as e: print("weight err", e)
    print("inv props: friendly=%s cap=%s wcap=%s" % (
        inv.get_editor_property("inventory_friendly_name"),
        inv.get_editor_property("capacity"),
        inv.get_editor_property("weight_capacity")))
else:
    print("NO inventory comp after add")

# chest-ify the existing Cube mesh proportions
if smc:
    try:
        smc.set_editor_property("relative_scale3d", unreal.Vector(0.8, 0.6, 0.5))
        print("mesh:", smc.get_name(), "scale set; static_mesh=", smc.get_editor_property("static_mesh"))
    except Exception as e: print("scale err", e)

# variables
bel = unreal.BlueprintEditorLibrary
existing_vars = set()
try:
    for v in bel.get_blueprint_variables(bp) if hasattr(bel,'get_blueprint_variables') else []:
        existing_vars.add(str(v))
except Exception:
    pass

def add_int_var(name):
    try:
        pt = unreal.EdGraphPinType()
        pt.import_text('(PinCategory="int")')
        bel.add_member_variable(bp, name, pt)
        try: bel.set_blueprint_variable_instance_editable(bp, name, True)
        except Exception as e: print("  instedit err", name, e)
        print("  var added", name)
    except Exception as e:
        print("  var FAIL", name, e)

for v in ["MinGold","MaxGold","MinPotions","MaxPotions"]:
    add_int_var(v)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_asset(DST)

# defaults on CDO
cdo = unreal.get_default_object(bp.generated_class())
for k,val in {"MinGold":30,"MaxGold":80,"MinPotions":1,"MaxPotions":2}.items():
    try:
        cdo.set_editor_property(k, val); print("  default", k, "=", cdo.get_editor_property(k))
    except Exception as e: print("  default FAIL", k, e)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_asset(DST)
print("DONE")
