import unreal

# __CONFIG__ is replaced per-run
NAME = "BP_Item_Gold"
PROPS = [("stackable",True),("max_stack_size",999999),("weight",0.01),("base_value",1),("display_name","Gold"),("description","Coin clipped and blackened by a hundred dead hands. It spends all the same.")]
THUMB = None

EAL = unreal.EditorAssetLibrary
PKG = "/Game/Inventory/Items"
PATH = PKG + "/" + NAME

if not EAL.does_directory_exist(PKG):
    EAL.make_directory(PKG)

if EAL.does_asset_exist(PATH):
    unreal.log_warning("NEXUS_ITEM %s ALREADY EXISTS - skipped create" % NAME)
    bp = EAL.load_asset(PATH)
else:
    try:
        factory = unreal.NarrativeItemBlueprintFactory()
    except AttributeError:
        factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.NarrativeItem)
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PKG, None, factory)

if not bp:
    unreal.log_error("NEXUS_ITEM %s CREATE FAILED" % NAME)
else:
    cls = EAL.load_blueprint_class(PATH)
    cdo = unreal.get_default_object(cls)
    errs = []
    for k, v in PROPS:
        try:
            cdo.set_editor_property(k, v)
        except Exception as e:
            errs.append("%s:%s" % (k, str(e)[:50]))
    if THUMB:
        tex = EAL.load_asset(THUMB)
        if tex:
            try:
                cdo.set_editor_property("thumbnail", tex)
            except Exception as e:
                errs.append("thumbnail:" + str(e)[:50])
        else:
            errs.append("thumbnail:load-failed")
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    saved = EAL.save_asset(PATH)
    unreal.log_warning("NEXUS_ITEM %s bp=%s class=%s saved=%s errs=%s" % (
        NAME, bp.get_class().get_name(), cls.get_name(), saved, errs if errs else "none"))
