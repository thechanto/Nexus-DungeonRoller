import unreal
ct = unreal.load_asset("/Game/GameplayAbilitySystem/Curves/CT_LinearTalents")
ok = unreal.AssetToolsHelpers.get_asset_tools().export_assets([ct.get_path_name()], r"C:/Users/chant/AppData/Local/Temp/ct_export")
print("CT export ok=%s" % ok)