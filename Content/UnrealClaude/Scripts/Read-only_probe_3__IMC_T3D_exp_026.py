import unreal, json, traceback
R = {}

try:
    at = unreal.AssetToolsHelpers.get_asset_tools()
    at.export_assets(['/Game/Input/IMC_Default',
                      '/Game/Input/IMC_MouseLook',
                      '/Game/GameplayAbilitySystem/Input/IMC_AbilityInputMapping'],
                     r'C:\Users\chant\AppData\Local\Temp\imc_export')
    R['imc_export'] = 'ok'
except Exception:
    R['imc_export_ERR'] = traceback.format_exc()[-400:]

try:
    iface = unreal.load_object(None, '/Game/GameplayAbilitySystem/BPI_Targeting.BPI_Targeting_C')
    for label, p in [('BP_NexusEnemy_Base', '/Game/GameplayAbilitySystem/Characters/BP_NexusEnemy_Base.BP_NexusEnemy_Base_C'),
                     ('BP_Enemy_Base', '/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C')]:
        try:
            c = unreal.load_object(None, p)
            o = unreal.get_default_object(c)
            R[label + '_implements'] = bool(unreal.SystemLibrary.does_implement_interface(o, iface))
        except Exception as e:
            R[label + '_ERR'] = str(e)[:120]
except Exception:
    R['iface_ERR'] = traceback.format_exc()[-300:]

with open(r'C:\Users\chant\AppData\Local\Temp\lockon_report3.json', 'w') as f:
    json.dump(R, f, indent=1)
print('LOCKON_PROBE3_DONE')
