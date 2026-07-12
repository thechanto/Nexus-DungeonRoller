import unreal, json, traceback

OUT = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/claude_stash_result.json"
BP_PATH = "/Game/ThirdPerson/Blueprints/BP_NexusPlayerState"
res = {"ok": False, "steps": []}

def log(k, v):
    res["steps"].append({k: v})

try:
    bp = unreal.EditorAssetLibrary.load_asset(BP_PATH)
    log("loaded_bp", bp.get_name())

    SDS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = SDS.k2_gather_subobject_data_for_blueprint(bp)

    def hname(h):
        d = SDS.k2_find_subobject_data_from_handle(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        return (o.get_name() if o else "<none>"), o

    existing = {}
    for h in handles:
        n, o = hname(h)
        existing[n] = o
    log("existing_subobjects", sorted(existing.keys()))

    inv_class = unreal.NarrativeInventoryComponent
    root = handles[0]

    # find or create Stash
    stash_obj = None
    for n, o in existing.items():
        if n.startswith("Stash"):
            stash_obj = o
    if stash_obj is None:
        params = unreal.AddNewSubobjectParams()
        params.parent_handle = root
        params.new_class = inv_class
        params.blueprint_context = bp
        new_h, fail = SDS.add_new_subobject(params)
        if fail and str(fail):
            log("add_fail", str(fail))
        SDS.rename_subobject(new_h, "Stash")
        _, stash_obj = hname(new_h)
        log("created", "Stash")
    else:
        log("already_present", "Stash")

    # set defaults
    stash_obj.set_editor_property("capacity", 50)
    stash_obj.set_editor_property("weight_capacity", 10000.0)
    stash_obj.set_editor_property("currency", 0)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(BP_PATH)

    # re-read off disk to verify
    unreal.EditorAssetLibrary.load_asset(BP_PATH)
    handles2 = SDS.k2_gather_subobject_data_for_blueprint(bp)
    verify = {}
    for h in handles2:
        n, o = hname(h)
        if o and isinstance(o, unreal.NarrativeInventoryComponent):
            verify[n] = {
                "class": o.get_class().get_name(),
                "capacity": o.get_editor_property("capacity"),
                "weight_capacity": o.get_editor_property("weight_capacity"),
                "currency": o.get_editor_property("currency"),
            }
    log("inventory_components", verify)
    log("all_subobjects_after", sorted([hname(h)[0] for h in handles2]))
    res["ok"] = True
except Exception as e:
    res["error"] = "".join(traceback.format_exception(type(e), e, e.__traceback__))

with open(OUT, "w") as f:
    json.dump(res, f, indent=2)
print("WROTE", OUT)
