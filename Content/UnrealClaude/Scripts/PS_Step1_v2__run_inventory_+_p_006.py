import unreal, json, traceback

OUT = r"C:\Users\chant\OneDrive\Documents\Unreal Projects\Nexus-part-final 5.7\Saved\nexus_ps_step1_result.json"
res = {"steps": [], "errors": []}

def log(k, v):
    res["steps"].append({k: v})

def subobjects(bp):
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    out = []
    for h in sds.k2_gather_subobject_data_for_blueprint(bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.k2_find_subobject_data_from_handle(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        out.append((h, obj))
    return sds, out

try:
    PS_BP = "/Game/ThirdPerson/Blueprints/BP_NexusPlayerState"
    GM_BP = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode"

    inv_class = unreal.NarrativeInventoryComponent
    inv_name = inv_class.static_class().get_name()
    log("inv_class", inv_class.static_class().get_path_name())

    bp = unreal.EditorAssetLibrary.load_asset(PS_BP)
    sds, subs = subobjects(bp)
    log("existing_subobjects", [o.get_class().get_name() for _, o in subs if o])

    have = [o for _, o in subs if o and o.get_class().get_name() == inv_name]
    if have:
        log("already_present", True)
    else:
        params = unreal.AddNewSubobjectParams(
            parent_handle=subs[0][0],
            new_class=inv_class,
            blueprint_context=bp)
        new_handle, fail = sds.add_new_subobject(params)
        if fail and not fail.is_empty():
            raise Exception("add_new_subobject failed: %s" % fail)
        sds.rename_subobject(new_handle, "RunInventory")
        log("added_component", "RunInventory")

    # resolve template
    sds, subs = subobjects(bp)
    tmpl = None
    for _, o in subs:
        if o and o.get_class().get_name() == inv_name:
            tmpl = o
    if tmpl is None:
        raise Exception("template not found after add")
    log("template_name", tmpl.get_name())

    before = {}
    for p in ["capacity", "weight_capacity", "currency"]:
        try:
            before[p] = str(tmpl.get_editor_property(p))
        except Exception as e:
            before[p] = "ERR: %s" % e
    log("defaults_before", before)

    tmpl.set_editor_property("capacity", 12)
    tmpl.set_editor_property("weight_capacity", 40.0)
    tmpl.set_editor_property("currency", 0)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(PS_BP)

    # verify by re-reading template from disk-backed asset
    bp2 = unreal.EditorAssetLibrary.load_asset(PS_BP)
    _, subs2 = subobjects(bp2)
    verify = {}
    for _, o in subs2:
        if o and o.get_class().get_name() == inv_name:
            verify = {
                "name": o.get_name(),
                "capacity": str(o.get_editor_property("capacity")),
                "weight_capacity": str(o.get_editor_property("weight_capacity")),
                "currency": str(o.get_editor_property("currency")),
            }
    log("verify_component", verify)

    # --- GameMode: set ONLY player_state_class ---
    ps_class = unreal.EditorAssetLibrary.load_blueprint_class(PS_BP)
    gm_bp = unreal.EditorAssetLibrary.load_asset(GM_BP)
    gm_cdo = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(GM_BP))
    fields = ["default_pawn_class", "player_controller_class", "game_state_class",
              "hud_class", "player_state_class", "spectator_class"]
    gm_before = {f: str(gm_cdo.get_editor_property(f)) for f in fields}
    log("gamemode_before", gm_before)

    gm_cdo.set_editor_property("player_state_class", ps_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(gm_bp)
    unreal.EditorAssetLibrary.save_asset(GM_BP)

    gm_cdo2 = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(GM_BP))
    gm_after = {f: str(gm_cdo2.get_editor_property(f)) for f in fields}
    log("gamemode_after", gm_after)
    log("changed_fields", [f for f in fields if gm_before[f] != gm_after[f]])

    res["ok"] = True
except Exception:
    res["ok"] = False
    res["errors"].append(traceback.format_exc())

with open(OUT, "w") as f:
    json.dump(res, f, indent=2)
print("WROTE " + OUT)
