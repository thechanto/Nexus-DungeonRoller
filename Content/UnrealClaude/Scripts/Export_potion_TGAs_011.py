import unreal

OUT = r"C:\Users\chant\AppData\Local\Temp\nexus_items\tga"
NUMS = [1, 2, 10, 13, 19, 22, 25, 28, 40, 55, 70, 100]
paths = ["/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_%d_1024x1024" % n for n in NUMS]
paths.append("/Game/StarterContent/Textures/T_Metal_Gold_D")

loaded = [p for p in paths if unreal.EditorAssetLibrary.does_asset_exist(p)]
unreal.AssetToolsHelpers.get_asset_tools().export_assets(loaded, OUT)
unreal.log("TGA_EXPORT requested=%d existing=%d" % (len(paths), len(loaded)))
