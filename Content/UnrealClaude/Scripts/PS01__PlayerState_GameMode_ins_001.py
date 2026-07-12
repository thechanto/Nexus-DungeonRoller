import unreal, json

res = {}

GM_PROPS = ["game_state_class", "player_state_class", "default_pawn_class",
            "player_controller_class", "hud_class", "spectator_class",
            "game_session_class"]

def cdo_probe(bp_path):
    d = {}
    try:
        cls = unreal.EditorAssetLibrary.load_blueprint_class(bp_path)
    except Exception as e:
        return {"__load_error__": str(e)}
    cdo = unreal.get_default_object(cls)
    for p in GM_PROPS:
        try:
            v = cdo.get_editor_property(p)
            d[p] = v.get_path_name() if v else None
        except Exception as e:
            d[p] = "ERR " + str(e)
    return d

res["BP_ThirdPersonGameMode"] = cdo_probe("/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode")
res["BP_MainMenuGameMode"] = cdo_probe("/Game/Maps/BP_MainMenuGameMode")

# --- WorldSettings GameMode override per level (read-only; does NOT open the map) ---
def ws_probe(lvl):
    try:
        unreal.EditorAssetLibrary.load_asset(lvl)
    except Exception as e:
        return {"__load_error__": str(e)}
    short = lvl.split("/")[-1]
    for cand in ["WorldSettings_0", "WorldSettings_1", "WorldSettings", "WorldSettings_2"]:
        path = "%s.%s:PersistentLevel.%s" % (lvl, short, cand)
        try:
            o = unreal.load_object(None, path)
        except Exception:
            continue
        if o:
            try:
                gm = o.get_editor_property("default_game_mode")
            except Exception as e:
                return {"ws": cand, "err": str(e)}
            return {"ws": cand, "default_game_mode_override": gm.get_path_name() if gm else None}
    return {"__error__": "WorldSettings subobject not found"}

for lvl in ["/Game/ThirdPerson/Lvl_ThirdPerson", "/Game/ThirdPerson/Lvl_Boss", "/Game/Maps/Lvl_MainMenu"]:
    res["WS " + lvl] = ws_probe(lvl)

# --- Any PlayerState-derived class at all (C++ or BP)? ---
ps_derived = []
for name in dir(unreal):
    try:
        c = getattr(unreal, name)
        if isinstance(c, type) and issubclass(c, unreal.PlayerState) and c is not unreal.PlayerState:
            ps_derived.append("native:" + name)
    except Exception:
        pass
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for ad in ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/Engine", "Blueprint"), True):
    pc = ad.get_tag_value("ParentClass") or ""
    if "PlayerState" in str(pc):
        ps_derived.append("bp:" + str(ad.package_name) + " -> " + str(pc))
res["playerstate_derived_classes"] = ps_derived

# --- Narrative inventory symbols exposed to python ---
res["inventory_symbols"] = sorted([n for n in dir(unreal) if "Inventory" in n])[:40]

# --- BP_NexusPlayer components (does it ALREADY have an inventory component?) ---
try:
    bp = unreal.EditorAssetLibrary.load_asset("/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer")
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sds.k2_gather_subobject_data_for_blueprint(bp)
    comps = []
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if o:
            comps.append(o.get_name() + " : " + o.get_class().get_name())
    res["BP_NexusPlayer_components"] = comps
except Exception as e:
    res["BP_NexusPlayer_components"] = "ERR " + str(e)

unreal.log("PS01_RESULT_BEGIN")
unreal.log(json.dumps(res, indent=2, default=str))
unreal.log("PS01_RESULT_END")
