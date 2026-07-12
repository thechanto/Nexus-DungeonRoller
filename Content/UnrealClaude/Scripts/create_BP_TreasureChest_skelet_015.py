import unreal

SRC = "/Game/Test/BP_InteractionTest"
DST_DIR = "/Game/Interactables"
DST_NAME = "BP_TreasureChest"
DST = DST_DIR + "/" + DST_NAME

at = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary

# 1. Duplicate (skip if exists)
if eal.does_asset_exist(DST):
    print("EXISTS:", DST)
    bp = eal.load_asset(DST)
else:
    bp = at.duplicate_asset(DST_NAME, DST_DIR, eal.load_asset(SRC))
    print("DUPLICATED ->", DST, bp)

bp = eal.load_asset(DST)
print("BP:", bp)

SDS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = SDS.k2_gather_subobject_data_for_blueprint(bp)
root = handles[0]

def hname(h):
    d = SDS.k2_find_subobject_data_from_handle(h)
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
    return o

print("--- existing components ---")
existing = {}
for h in handles:
    o = hname(h)
    if o:
        existing[o.get_name()] = (h, o)
        print("  ", o.get_name(), o.get_class().get_name())

def add_comp(cls, desired_name):
    for nm,(h,o) in existing.items():
        if o.get_class() == cls or o.get_class().get_name()==cls.get_name():
            # already has one of this type? still add if name differs
            pass
    params = unreal.AddNewSubobjectParams(parent_handle=root, new_class=cls, blueprint_context=bp)
    new_h, fail = SDS.add_new_subobject(params)
    if not fail.is_empty():
        print("  add FAIL", desired_name, fail)
        return None
    SDS.rename_subobject(new_h, unreal.Text.from_string(desired_name))
    d = SDS.k2_find_subobject_data_from_handle(new_h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
    print("  added", desired_name, "->", obj.get_name() if obj else None)
    return obj

# 2. StaticMesh component (only if none present)
has_sm = any(o.get_class()==unreal.StaticMeshComponent for _,(h,o) in existing.items())
print("has StaticMeshComponent:", has_sm)
sm_obj = None
if not has_sm:
    sm_obj = add_comp(unreal.StaticMeshComponent, "ChestMesh")

# 3. NarrativeInventory component
has_inv = any(o.get_class()==unreal.NarrativeInventoryComponent for _,(h,o) in existing.items())
print("has NarrativeInventoryComponent:", has_inv)
inv_obj = None
if not has_inv:
    inv_obj = add_comp(unreal.NarrativeInventoryComponent, "ChestInventory")

# re-gather to get fresh template objects
handles = SDS.k2_gather_subobject_data_for_blueprint(bp)
def find_obj(pred):
    for h in handles:
        o = hname(h)
        if o and pred(o):
            return o
    return None

# 4. Set mesh
cube = eal.load_asset("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube")
smc = find_obj(lambda o: o.get_class()==unreal.StaticMeshComponent)
if smc and cube:
    smc.set_editor_property("static_mesh", cube)
    smc.set_editor_property("relative_scale3d", unreal.Vector(0.8, 0.6, 0.5))
    print("mesh set on", smc.get_name())
else:
    print("NO static mesh comp or cube missing", smc, cube)

# 5. Inventory props
inv = find_obj(lambda o: o.get_class()==unreal.NarrativeInventoryComponent)
if inv:
    try:
        inv.set_editor_property("inventory_friendly_name", unreal.Text.from_string("Chest"))
    except Exception as e:
        print("friendly name err", e)
    inv.set_editor_property("capacity", 10)
    try:
        inv.set_editor_property("weight_capacity", 1000.0)
    except Exception as e:
        print("weight err", e)
    print("inventory props set: friendly=%s cap=%s" % (
        inv.get_editor_property("inventory_friendly_name"),
        inv.get_editor_property("capacity")))
else:
    print("NO inventory comp")

# 6. Variables (instance-editable ranges)
bel = unreal.BlueprintEditorLibrary
def add_int_var(name):
    try:
        pt = unreal.EdGraphPinType()
        pt.import_text('(PinCategory="int")')
        bel.add_member_variable(bp, name, pt)
        try:
            bel.set_blueprint_variable_instance_editable(bp, name, True)
        except Exception as e:
            print("  instedit err", name, e)
        print("  var added", name)
    except Exception as e:
        print("  var FAIL", name, e)

for v in ["MinGold","MaxGold","MinPotions","MaxPotions"]:
    add_int_var(v)

# 7. Compile + save
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_asset(DST)
print("SAVED", DST)

# 8. set var defaults on CDO after compile
cdo = unreal.get_default_object(bp.generated_class())
defaults = {"MinGold":30,"MaxGold":80,"MinPotions":1,"MaxPotions":2}
for k,val in defaults.items():
    try:
        cdo.set_editor_property(k, val)
        print("  default", k, "=", cdo.get_editor_property(k))
    except Exception as e:
        print("  default FAIL", k, e)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_asset(DST)
print("DONE")
