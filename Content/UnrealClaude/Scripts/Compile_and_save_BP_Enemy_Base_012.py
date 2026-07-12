import unreal

MARK = "LOOT11"
BP_PATH = "/Game/Enemies/BP_Enemy_Base"
bp = unreal.load_asset(BP_PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH)
unreal.log(MARK + ": compiled + save_asset -> " + str(saved))
