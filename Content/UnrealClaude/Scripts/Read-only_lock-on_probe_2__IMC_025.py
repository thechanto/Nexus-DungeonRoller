import unreal, json, traceback
R = {}

# ---------- 1. IMC mappings via T3D export ----------
exp_dir = r'C:\Users\chant\AppData\Local\Temp\imc_export'
try:
    at = unreal.AssetToolsHelpers.get_asset_tools()
    assets = [unreal.load_asset(p) for p in [
        '/Game/Input/IMC_Default',
        '/Game/Input/IMC_MouseLook',
        '/Game/GameplayAbilitySystem/Input/IMC_AbilityInputMapping']]
    at.export_assets([a for a in assets if a], exp_dir)
    R['imc_export'] = 'ok'
except Exception:
    R['imc_export_ERR'] = traceback.format_exc()[-400:]

# ---------- 2. BP_NexusEnemy_Base ----------
try:
    c = unreal.load_object(None, '/Game/GameplayAbilitySystem/Characters/BP_NexusEnemy_Base.BP_NexusEnemy_Base_C')
    o = unreal.get_default_object(c)
    iface = unreal.load_object(None, '/Game/GameplayAbilitySystem/BPI_Targeting.BPI_Targeting_C')
    R['BP_NexusEnemy_Base'] = {
        'parent': c.get_editor_property('super_struct').get_name() if c.get_editor_property('super_struct') else '?',
        'implements_BPI_Targeting': bool(unreal.SystemLibrary.does_implement_interface(o, iface)),
    }
except Exception:
    R['nexusenemy_ERR'] = traceback.format_exc()[-400:]

# is BP_Enemy_Base a child of BP_NexusEnemy_Base or of C++ ANexusEnemyBase?
try:
    c2 = unreal.load_object(None, '/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C')
    chain = []
    cur = c2
    for _ in range(8):
        if not cur: break
        chain.append(cur.get_name())
        cur = cur.get_editor_property('super_struct') if hasattr(cur, 'get_editor_property') else None
        try:
            cur = chain and unreal.load_object(None, '') # placeholder never used
        except Exception:
            pass
        break
    # simpler: walk with get_super_class via unreal.SystemLibrary? use class hierarchy via python
    names = []
    k = c2
    while k:
        names.append(k.get_name())
        try:
            k = k.get_editor_property('super_struct')
        except Exception:
            k = None
    R['BP_Enemy_Base_chain'] = names
    iface = unreal.load_object(None, '/Game/GameplayAbilitySystem/BPI_Targeting.BPI_Targeting_C')
    o2 = unreal.get_default_object(c2)
    R['BP_Enemy_Base_implements'] = bool(unreal.SystemLibrary.does_implement_interface(o2, iface))
except Exception:
    R['chain_ERR'] = traceback.format_exc()[-400:]

# ---------- 3. BP_Enemy_Base component list (indicator anchor / health bar widget?) ----------
try:
    bp = unreal.load_asset('/Game/Enemies/BP_Enemy_Base')
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    comps = []
    for h in sds.k2_gather_subobject_data_for_blueprint(bp):
        data = sds.k2_find_subobject_data_from_handle(h)
        obj = lib.get_object(data)
        if obj:
            try:
                vname = str(lib.get_variable_name(data))
            except Exception:
                vname = obj.get_name()
            comps.append(vname + ' : ' + obj.get_class().get_name())
    R['enemy_base_components'] = comps
except Exception:
    R['enemycomp_ERR'] = traceback.format_exc()[-400:]

out_path = r'C:\Users\chant\AppData\Local\Temp\lockon_report2.json'
with open(out_path, 'w') as f:
    json.dump(R, f, indent=1)
print('LOCKON_PROBE2_DONE')
