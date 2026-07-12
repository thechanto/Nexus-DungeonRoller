import unreal

BP_PATH = "/Game/Interactables/BP_TreasureChest"

def mark(k, *v):
    unreal.log("CHESTMARK|%s|%s" % (k, " ".join(str(x) for x in v)))

bp = unreal.load_asset(BP_PATH)
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sds.k2_gather_subobject_data_for_blueprint(bp)

sm_comp = None
comps = []
for h in handles:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if obj is None:
        continue
    comps.append("%s : %s" % (obj.get_name(), obj.get_class().get_name()))
    if isinstance(obj, unreal.StaticMeshComponent) and sm_comp is None:
        sm_comp = obj
mark("COMPS", " || ".join(comps))

if sm_comp is None:
    mark("SM_COMP", "NONE_FOUND")
else:
    mark("SM_COMP", sm_comp.get_name())
    cur = sm_comp.get_editor_property("static_mesh")
    mark("SM_CURRENT", cur.get_name() if cur else "None")
    if cur is None:
        mesh = None
        for cand in ["/Game/LevelPrototyping/Meshes/SM_Cube",
                     "/Game/StarterContent/Shapes/Shape_Cube",
                     "/Engine/BasicShapes/Cube"]:
            m = unreal.load_asset(cand)
            if m:
                mesh = m
                mark("MESH_PICK", cand)
                break
        if mesh:
            sm_comp.set_editor_property("static_mesh", mesh)
            sm_comp.set_editor_property("relative_scale3d", unreal.Vector(1.2, 0.9, 0.6))
            mark("MESH_SET", sm_comp.get_editor_property("static_mesh").get_name())
        else:
            mark("MESH_SET", "NO_CANDIDATE_LOADED")
    else:
        mark("MESH_KEPT", cur.get_name())

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
mark("SAVED", saved)
mark("DONE", "ok")
