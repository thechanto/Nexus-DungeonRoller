import unreal
# save the IMC mapping changes from the previous step
unreal.EditorAssetLibrary.save_asset("/Game/Input/IMC_Default")
# duplicate the interact prompt as the indicator base
if unreal.EditorAssetLibrary.does_asset_exist("/Game/Widgets/W_LockOnIndicator"):
    unreal.log("LOCKON03: already exists")
else:
    dup = unreal.EditorAssetLibrary.duplicate_asset("/Game/Widgets/W_InteractPrompt", "/Game/Widgets/W_LockOnIndicator")
    unreal.log("LOCKON03: dup=" + str(dup))
# export to T3D so we can see the tree
unreal.AssetTools = unreal.AssetToolsHelpers.get_asset_tools()
unreal.AssetTools.export_assets(["/Game/Widgets/W_LockOnIndicator"], "C:/Users/chant/AppData/Local/Temp/lockon_export")
unreal.log("LOCKON03: exported")
