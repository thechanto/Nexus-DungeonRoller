import unreal, os
out_dir = os.path.join(os.environ.get("TEMP", "C:/Temp"), "nexus_t3d_abscreen")
os.makedirs(out_dir, exist_ok=True)
asset = unreal.load_object(None, "/Game/Widgets/W_AbilitiesScreen.W_AbilitiesScreen")
unreal.AssetToolsHelpers.get_asset_tools().export_assets([asset], out_dir)
print("EXPORTED_TO:" + out_dir)