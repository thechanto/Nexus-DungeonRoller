import unreal, json, traceback

OUT = r"C:\Users\chant\OneDrive\Documents\Unreal Projects\Nexus-part-final 5.7\Saved\nexus_ps_step1_result.json"
res = {"steps": [], "errors": [], "stash_probe": {}}

def log(k, v):
    res["steps"].append({k: v})

SDS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)

def subobjects(bp):
    out = []
    for h in SDS.k2_gather_subobject_data_for_blueprint(bp):
        d = SDS.k2_find_subobject_data_from_handle(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        out.append((h, obj))
    return out

try:
    PS_BP = "/Game/ThirdPerson/Blueprints/BP_NexusPlayerState"
    GM_BP = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode"

    inv_class = unreal.NarrativeInventoryComponent
    inv_name = inv_class.static_class().get_name()

    bp = unreal.EditorAssetLibrary.load_asset(PS_BP)
    subs = subobjects(bp)
    log("existing_subobjects", [o.get_class().get_name() for _, o in subs if o])

    have = [o for _, o in subs if o and o.get_class().get_name() == inv_name]
    if have:
        log("already_present", True)
    else:
        params = unreal.AddNewSubobjectParams(
            parent_handle=subs[0][0], new_class=inv_class, blueprint_context=bp)
        new_handle, fail = SDS.add_new_subobject(params)
        if fail and not fail.is_empty():
            raise Exception("add_new_subobject failed: %s" % fail)
        SDS.rename_subobject(new_handle, "RunInventory")
        log("added_component", "RunInventory")

    subs = subobjects(bp)
    tmpl = None
    for _, o in subs:
        if o and o.get_class().get_name() == inv_name:
            tmpl = o
    if tmpl is None:
        raise Exception("template not found after add")

    before = {}
    for p in ["capacity", "weight_capacity", "currency"]:
        try:
            before[p] = str(tmpl.get_editor_property(p))
        except Exception as e:
            before[p] = "NOT_EDITOR_EXPOSED"
    log("defaults_before", before)

    tmpl.set_editor_property("capacity", 12)
    tmpl.set_editor_property("weight_capacity", 40.0)
    try:
        tmpl.set_editor_property("currency", 0)
        log("currency_set", "ok")
    except Exception:
        log("currency_set", "NOT_EDITOR_EXPOSED (defaults to 0)")

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(PS_BP)

    bp2 = unreal.EditorAssetLibrary.load_asset(PS_BP)
    verify = {}
    for _, o in subobjects(bp2):
        if o and o.get_class().get_name() == inv_name:
            verify = {"name": o.get_name(),
                      "capacity": str(o.get_editor_property("capacity")),
                      "weight_capacity": str(o.get_editor_property("weight_capacity"))}
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

# ---------- STASH / SAVE PROBE (read-only) ----------
try:
    sp = res["stash_probe"]
    sp["unreal_names_inventory_save"] = sorted(
        [n for n in dir(unreal) if "InventorySave" in n or
         ("Inventory" in n and "Save" in n)])
    sp["narrative_save_classes"] = sorted(
        [n for n in dir(unreal) if "Narrative" in n and "Save" in n])
    for cn in ["InventorySaveGame", "NarrativeInventorySaveGame", "NarrativeSaveGame"]:
        if hasattr(unreal, cn):
            sp[cn + "_members"] = [m for m in dir(getattr(unreal, cn))
                                   if not m.startswith("_")]
    # signature-ish info for save/load
    sp["component_save_api"] = [m for m in dir(unreal.NarrativeInventoryComponent)
                                if m in ("save", "load", "delete_save",
                                         "inventory_friendly_name",
                                         "get_items", "try_add_item_from_class",
                                         "default_item_tables", "gave_default_items")]
    if hasattr(unreal, "InventoryFunctionLibrary"):
        sp["InventoryFunctionLibrary"] = [m for m in dir(unreal.InventoryFunctionLibrary)
                                          if not m.startswith("_")]
    sp["item_classes"] = sorted([n for n in dir(unreal)
                                 if "NarrativeItem" in n or n.endswith("ItemStack")])[:40]
except Exception:
    res["errors"].append("stash_probe: " + traceback.format_exc())

with open(OUT, "w") as f:
    json.dump(res, f, indent=2)
print("WROTE " + OUT)
