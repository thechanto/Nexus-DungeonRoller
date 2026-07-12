import unreal, json, traceback

OUT = r"C:\Users\chant\OneDrive\Documents\Unreal Projects\Nexus-part-final 5.7\Saved\nexus_ps_step1_result.json"
res = {"steps": [], "errors": []}

def log(k, v):
    res["steps"].append({k: v})

try:
    PS_BP = "/Game/ThirdPerson/Blueprints/BP_NexusPlayerState"
    GM_BP = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode"

    # --- locate the inventory component class ---
    inv_class = None
    for cand in ["NarrativeInventoryComponent", "InventoryComponent"]:
        if hasattr(unreal, cand):
            inv_class = getattr(unreal, cand)
            log("inv_class_attr", cand)
            break
    if inv_class is None:
        raise Exception("NarrativeInventoryComponent not found on unreal module")
    log("inv_class_path", unreal.SystemLibrary.get_path_name(inv_class))

    bp = unreal.EditorAssetLibrary.load_asset(PS_BP)
    log("ps_bp_loaded", str(bp))

    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sds.k2_gather_subobject_data_for_blueprint(bp)
    log("existing_subobject_count", len(handles))

    # skip if already present (idempotent)
    existing = []
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if obj:
            existing.append(obj.get_class().get_name())
    log("existing_subobjects", existing)

    inv_name = inv_class.get_default_object().get_class().get_name()
    if inv_name in existing:
        log("already_present", True)
        inv_handle = None
    else:
        params = unreal.AddNewSubobjectParams(
            parent_handle=handles[0],
            new_class=inv_class,
            blueprint_context=bp)
        inv_handle, fail = sds.add_new_subobject(params)
        if fail and str(fail):
            res["errors"].append("add_new_subobject: %s" % str(fail))
        sds.rename_subobject(inv_handle, "RunInventory")
        log("added_component", "RunInventory")

    # --- set defaults on the component template ---
    handles = sds.k2_gather_subobject_data_for_blueprint(bp)
    tmpl = None
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if obj and obj.get_class().get_name() == inv_name:
            tmpl = obj
    if tmpl is None:
        raise Exception("could not resolve inventory template after add")
    log("template", str(tmpl))

    # report every property we can see, so we set the right names
    props = {}
    for p in ["capacity", "weight_capacity", "currency"]:
        try:
            props[p] = tmpl.get_editor_property(p)
        except Exception as e:
            props[p] = "ERR: %s" % e
    log("defaults_before", {k: str(v) for k, v in props.items()})

    tmpl.set_editor_property("capacity", 12)
    tmpl.set_editor_property("weight_capacity", 40.0)
    tmpl.set_editor_property("currency", 0)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(PS_BP)

    # re-read from a freshly loaded CDO to prove it stuck
    ps_class = unreal.EditorAssetLibrary.load_blueprint_class(PS_BP)
    cdo = unreal.get_default_object(ps_class)
    verify = {}
    for c in cdo.get_components_by_class(inv_class):
        verify = {
            "name": c.get_name(),
            "capacity": c.get_editor_property("capacity"),
            "weight_capacity": c.get_editor_property("weight_capacity"),
            "currency": c.get_editor_property("currency"),
        }
    log("verify_component", {k: str(v) for k, v in verify.items()})

    # --- GameMode: set ONLY player_state_class ---
    gm_class = unreal.EditorAssetLibrary.load_blueprint_class(GM_BP)
    gm_cdo = unreal.get_default_object(gm_class)
    fields = ["default_pawn_class", "player_controller_class", "game_state_class",
              "hud_class", "player_state_class", "spectator_class"]
    before = {f: str(gm_cdo.get_editor_property(f)) for f in fields}
    log("gamemode_before", before)

    gm_cdo.set_editor_property("player_state_class", ps_class)
    gm_bp = unreal.EditorAssetLibrary.load_asset(GM_BP)
    unreal.BlueprintEditorLibrary.compile_blueprint(gm_bp)
    unreal.EditorAssetLibrary.save_asset(GM_BP)

    gm_class2 = unreal.EditorAssetLibrary.load_blueprint_class(GM_BP)
    gm_cdo2 = unreal.get_default_object(gm_class2)
    after = {f: str(gm_cdo2.get_editor_property(f)) for f in fields}
    log("gamemode_after", after)
    log("only_player_state_changed",
        [f for f in fields if before[f] != after[f]])

    res["ok"] = True
except Exception as e:
    res["ok"] = False
    res["errors"].append(traceback.format_exc())

with open(OUT, "w") as f:
    json.dump(res, f, indent=2)
print("WROTE " + OUT)
