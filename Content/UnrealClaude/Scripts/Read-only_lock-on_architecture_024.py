import unreal, json, traceback
R = {}

def props(obj, names):
    d = {}
    for n in names:
        try:
            d[n] = str(obj.get_editor_property(n))
        except Exception as e:
            d[n] = 'ERR:' + str(e)[:70]
    return d

# ---------- 1. Player component tree ----------
try:
    bp = unreal.load_asset('/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer')
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    handles = sds.k2_gather_subobject_data_for_blueprint(bp)
    comps = []
    spring = None; cam = None
    for h in handles:
        data = sds.k2_find_subobject_data_from_handle(h)
        obj = lib.get_object(data)
        if not obj:
            continue
        try:
            vname = str(lib.get_variable_name(data))
        except Exception:
            vname = obj.get_name()
        comps.append(vname + ' : ' + obj.get_class().get_name())
        if isinstance(obj, unreal.SpringArmComponent):
            spring = obj
        if isinstance(obj, unreal.CameraComponent):
            cam = obj
    R['player_components'] = comps
    if spring:
        R['springarm'] = props(spring, ['target_arm_length','socket_offset','target_offset',
            'use_pawn_control_rotation','enable_camera_lag','camera_lag_speed',
            'enable_camera_rotation_lag','camera_rotation_lag_speed','do_collision_test'])
    if cam:
        R['camera'] = props(cam, ['use_pawn_control_rotation','field_of_view'])
except Exception:
    R['player_components_ERR'] = traceback.format_exc()[-500:]

# ---------- 2. Pawn / movement rotation config ----------
try:
    gen = unreal.load_object(None, '/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer.BP_NexusPlayer_C')
    cdo = unreal.get_default_object(gen)
    R['pawn_rot'] = props(cdo, ['use_controller_rotation_yaw','use_controller_rotation_pitch','use_controller_rotation_roll'])
    cm = cdo.get_editor_property('character_movement')
    R['charmove'] = props(cm, ['orient_rotation_to_movement','use_controller_desired_rotation','rotation_rate','max_walk_speed'])
except Exception:
    R['pawn_ERR'] = traceback.format_exc()[-500:]

# ---------- 3. BPI_Targeting implementers + referencers ----------
try:
    iface = unreal.load_object(None, '/Game/GameplayAbilitySystem/BPI_Targeting.BPI_Targeting_C')
    impl = {}
    for label, p in [
        ('BP_NexusPlayer', '/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer.BP_NexusPlayer_C'),
        ('BP_Enemy_Base', '/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C'),
        ('BP_Enemy_Melee', '/Game/Enemies/MeleeEnemy/BP_Enemy_Melee.BP_Enemy_Melee_C'),
    ]:
        try:
            c = unreal.load_object(None, p)
            o = unreal.get_default_object(c)
            impl[label] = bool(unreal.SystemLibrary.does_implement_interface(o, iface))
        except Exception as e:
            impl[label] = 'ERR:' + str(e)[:80]
    R['implements_BPI_Targeting'] = impl
    R['BPI_Targeting_referencers'] = [str(x) for x in unreal.EditorAssetLibrary.find_package_referencers_for_asset('/Game/GameplayAbilitySystem/BPI_Targeting')]
except Exception:
    R['iface_ERR'] = traceback.format_exc()[-500:]

# ---------- 4. Input mapping dump ----------
def dump_imc(path):
    imc = unreal.load_asset(path)
    out = []
    for m in imc.get_editor_property('mappings'):
        act = m.get_editor_property('action')
        key = m.get_editor_property('key')
        out.append((act.get_name() if act else 'None') + ' <- ' + str(key.get_editor_property('key_name')))
    return out

for p in ['/Game/Input/IMC_Default', '/Game/Input/IMC_MouseLook', '/Game/GameplayAbilitySystem/Input/IMC_AbilityInputMapping']:
    try:
        R['imc_' + p.split('/')[-1]] = dump_imc(p)
    except Exception:
        R['imc_ERR_' + p] = traceback.format_exc()[-300:]

try:
    R['IA_TargetConfirm_referencers'] = [str(x) for x in unreal.EditorAssetLibrary.find_package_referencers_for_asset('/Game/Input/Actions/IA_TargetConfirm')]
except Exception:
    R['tc_ERR'] = traceback.format_exc()[-300:]

# ---------- 5. Enemy meshes + sockets/bones ----------
def enemy_sockets(label, cls_path):
    try:
        c = unreal.load_object(None, cls_path)
        o = unreal.get_default_object(c)
        mesh = o.get_editor_property('mesh')
        sk = mesh.get_editor_property('skeletal_mesh_asset') if mesh else None
        d = {'mesh_asset': sk.get_path_name() if sk else None}
        if sk:
            try:
                socks = [str(s.get_editor_property('socket_name')) for s in sk.get_editor_property('sockets')]
                d['mesh_sockets'] = socks
            except Exception as e:
                d['mesh_sockets'] = 'ERR:' + str(e)[:60]
            try:
                names = [str(n) for n in mesh.get_all_socket_names()]
                interesting = [n for n in names if any(k in n.lower() for k in ['head','neck','spine','chest','pelvis','root','socket'])]
                d['bones_and_sockets_filtered'] = interesting
                d['total_socket_bone_count'] = len(names)
            except Exception as e:
                d['bones_and_sockets_filtered'] = 'ERR:' + str(e)[:60]
        R['enemy_' + label] = d
    except Exception:
        R['enemy_' + label + '_ERR'] = traceback.format_exc()[-400:]

enemy_sockets('base', '/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C')
enemy_sockets('melee', '/Game/Enemies/MeleeEnemy/BP_Enemy_Melee.BP_Enemy_Melee_C')

# ---------- 6. S_CameraSettings struct members ----------
try:
    st = unreal.load_asset('/Game/GameplayAbilitySystem/S_CameraSettings')
    R['S_CameraSettings_found_at_default_path'] = bool(st)
except Exception:
    pass
try:
    gc = unreal.load_object(None, '/Game/GameplayAbilitySystem/Cues/GC_Actor_TargetingCamera.GC_Actor_TargetingCamera_C')
    gco = unreal.get_default_object(gc)
    ts = gco.get_editor_property('TargetingCameraSettings')
    R['S_CameraSettings_export'] = ts.export_text()
except Exception:
    R['camsettings_ERR'] = traceback.format_exc()[-400:]

out_path = r'C:\Users\chant\AppData\Local\Temp\lockon_report.json'
with open(out_path, 'w') as f:
    json.dump(R, f, indent=1)
print('LOCKON_PROBE_DONE -> ' + out_path)
