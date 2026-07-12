import unreal

MARK = "LOOT07"
BP_PATH = "/Game/Loot/BP_LootPickup"
bp = unreal.load_asset(BP_PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH)
unreal.log(MARK + ": compiled + save_asset -> " + str(saved))
