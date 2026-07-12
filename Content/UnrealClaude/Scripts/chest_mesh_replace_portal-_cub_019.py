import unreal

BP_PATH = "/Game/Interactables/BP_TreasureChest"

def mark(k, *v):
    unreal.log("CHESTMARK|%s|%s" % (k, " ".join(str(x) for x in v)))

bp = unreal.load_asset(BP_PATH)
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sds.k2_gather_subobject_data_for_blueprint(bp)

sm_comp = None
for h in handles:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if obj and isinstance(obj, unreal.StaticMeshComponent):
        sm_comp = obj
        break

mesh = None
for cand in ["/Game/LevelPrototyping/Meshes/SM_Cube",
             "/Game/StarterContent/Shapes/Shape_Cube",
             "/Engine/BasicShapes/Cube"]:
    m = unreal.load_asset(cand)
    if m:
        mesh = m
        mark("MESH_PICK", cand)
        break

if sm_comp and mesh:
    sm_comp.set_editor_property("static_mesh", mesh)
    sm_comp.set_editor_property("relative_scale3d", unreal.Vector(1.2, 0.7, 0.7))
    sm_comp.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 35.0))
    mark("MESH_SET", sm_comp.get_editor_property("static_mesh").get_name())
    mark("SCALE", sm_comp.get_editor_property("relative_scale3d"))
else:
    mark("FAIL", "sm_comp=%s mesh=%s" % (sm_comp, mesh))

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
mark("SAVED", saved)
mark("DONE", "ok")
