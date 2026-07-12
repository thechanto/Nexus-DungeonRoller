import unreal

MARK = "LOOT09"
HUD_PATH = "/Game/Widgets/W_PlayerHUD"

t = unreal.load_object(None, HUD_PATH + ".W_PlayerHUD:WidgetTree")
unreal.log(MARK + ": WidgetTree subobject = " + str(t))
if t:
    try:
        rw = t.get_editor_property("root_widget")
        unreal.log(MARK + ": root_widget = " + str(rw))
    except Exception as e:
        unreal.log(MARK + ": root_widget failed: " + str(e))

has_lib = hasattr(unreal, "WidgetBlueprintEditorLibrary")
unreal.log(MARK + ": WidgetBlueprintEditorLibrary exists = " + str(has_lib))
if has_lib:
    unreal.log(MARK + ": members = " + ", ".join([n for n in dir(unreal.WidgetBlueprintEditorLibrary) if not n.startswith("_")]))

# T3D export for tree structure
wb = unreal.load_asset(HUD_PATH)
at = unreal.AssetToolsHelpers.get_asset_tools()
at.export_assets([HUD_PATH], "C:/Users/chant/AppData/Local/Temp/nexus_loot/export")
unreal.log(MARK + ": exported HUD to T3D")
