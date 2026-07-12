import unreal, os, re

def try_load(p):
    try:
        c = unreal.load_object(None, p)
        return c
    except Exception as e:
        return "ERR:%s" % e

hud = try_load("/NarrativeCommonUI/Widgets/WBP_NarrativeHUD.WBP_NarrativeHUD")
loot = try_load("/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting")
print("HUD asset:", hud)
print("LOOT asset:", loot)

# export both to T3D and scan for looting refs
outdir = unreal.Paths.project_saved_dir() + "/T3D_loot"
try:
    unreal.EditorAssetLibrary.does_asset_exist("/NarrativeCommonUI/Widgets/WBP_NarrativeHUD")
except Exception as e:
    print("exist err", e)

at = unreal.AssetToolsHelpers.get_asset_tools()
for path,label in [("/NarrativeCommonUI/Widgets/WBP_NarrativeHUD","HUD"),
                   ("/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting","LOOTMENU")]:
    try:
        a = unreal.EditorAssetLibrary.load_asset(path)
        et = unreal.AssetExportTask()
        et.object = a
        et.filename = os.path.join(outdir, label+".t3d").replace("/","\\")
        et.automated = True
        et.write_empty_files = False
        et.replace_identical = True
        et.prompt = False
        ok = unreal.Exporter.run_asset_export_task(et)
        print(label, "export", ok, et.filename)
    except Exception as e:
        print(label, "export ERR", e)
