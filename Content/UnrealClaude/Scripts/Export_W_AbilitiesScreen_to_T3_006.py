import unreal, os
out_dir = os.path.join(os.environ.get("TEMP", "C:/Temp"), "nexus_t3d_abscreen")
os.makedirs(out_dir, exist_ok=True)
unreal.AssetToolsHelpers.get_asset_tools().export_assets(["/Game/Widgets/W_AbilitiesScreen"], out_dir)
print("EXPORTED_TO:" + out_dir)