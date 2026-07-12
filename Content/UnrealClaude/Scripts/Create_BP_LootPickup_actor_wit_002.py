import unreal

BP_PATH = "/Game/Loot/BP_LootPickup"
MARK = "LOOT01"

if unreal.EditorAssetLibrary.does_asset_exist(BP_PATH):
    unreal.log(MARK + ": already exists, configuring components only")
    bp = unreal.load_asset(BP_PATH)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Actor)
    at = unreal.AssetToolsHelpers.get_asset_tools()
    bp = at.create_asset("BP_LootPickup", "/Game/Loot", None, factory)
    unreal.log(MARK + ": created " + str(bp.get_path_name()))

sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
handles = sds.k2_gather_subobject_data_for_blueprint(bp)
unreal.log(MARK + ": %d existing subobject handles" % len(handles))

# find the scene-root handle (DefaultSceneRoot) and the actor handle
actor_handle = handles[0]
scene_root_handle = handles[1] if len(handles) > 1 else handles[0]
for h in handles:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = lib.get_object(data)
    unreal.log(MARK + ": existing subobject -> " + str(obj))

def add_comp(cls, name, parent_handle):
    params = unreal.AddNewSubobjectParams(
        parent_handle=parent_handle, new_class=cls, blueprint_context=bp)
    handle, fail = sds.add_new_subobject(params)
    fail_str = str(fail)
    if fail_str and fail_str != "None" and len(fail_str) > 0 and fail_str.strip():
        unreal.log(MARK + ": add %s fail reason: '%s'" % (name, fail_str))
    ok = sds.rename_subobject(handle, name)
    data = sds.k2_find_subobject_data_from_handle(handle)
    obj = lib.get_object(data)
    unreal.log(MARK + ": added %s -> %s (rename ok=%s)" % (name, str(obj), str(ok)))
    return handle, obj

# 1) sphere collision, radius 60 (shape components default to OverlapAllDynamic)
sphere_handle, sphere = add_comp(unreal.SphereComponent, "PickupSphere", scene_root_handle)
sphere.set_editor_property("sphere_radius", 60.0)

# 2) visual mesh under the sphere: gold-coin defaults, no collision
mesh_handle, mesh = add_comp(unreal.StaticMeshComponent, "PickupMesh", sphere_handle)
coin = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Cylinder")
gold_mat = unreal.load_asset("/Game/StarterContent/Materials/M_Metal_Gold")
mesh.set_editor_property("static_mesh", coin)
mesh.set_editor_property("override_materials", [gold_mat])
mesh.set_editor_property("relative_scale3d", unreal.Vector(0.3, 0.3, 0.06))
try:
    mesh.set_collision_profile_name("NoCollision")
    unreal.log(MARK + ": mesh collision -> NoCollision via setter")
except Exception as e:
    unreal.log(MARK + ": set_collision_profile_name failed: " + str(e))

# 3) visual spin
rot_handle, rot = add_comp(unreal.RotatingMovementComponent, "Rotator", actor_handle)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH)
unreal.log(MARK + ": compiled, save_asset -> " + str(saved))
