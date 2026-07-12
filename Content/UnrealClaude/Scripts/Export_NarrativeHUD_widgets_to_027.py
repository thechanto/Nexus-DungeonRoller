import unreal
outdir = r"C:\Users\chant\AppData\Local\Temp\nexus_t3d"
paths = ["/NarrativeCommonUI/Widgets/WBP_NarrativeHUD.WBP_NarrativeHUD",
         "/NarrativeCommonUI/Widgets/WBP_NarrativeMenu.WBP_NarrativeMenu"]
unreal.AssetToolsHelpers.get_asset_tools().export_assets(paths, outdir)
print("EXPORT DONE")
