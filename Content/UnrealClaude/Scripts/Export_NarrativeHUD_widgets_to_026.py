import unreal, os
outdir = r"C:\Users\chant\AppData\Local\Temp\nexus_t3d"
os.makedirs(outdir, exist_ok=True)
paths = ["/NarrativeCommonUI/Widgets/WBP_NarrativeHUD.WBP_NarrativeHUD",
         "/NarrativeCommonUI/Widgets/WBP_NarrativeMenu.WBP_NarrativeMenu"]
assets = [unreal.load_asset(p) for p in paths]
unreal.AssetToolsHelpers.get_asset_tools().export_assets(assets, outdir)
print("EXPORTED to", outdir)
