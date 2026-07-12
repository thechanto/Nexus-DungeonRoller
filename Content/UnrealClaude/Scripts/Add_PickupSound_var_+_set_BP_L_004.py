import unreal

MARK = "LOOT03"
BP_PATH = "/Game/Loot/BP_LootPickup"
bp = unreal.load_asset(BP_PATH)

pt = unreal.EdGraphPinType()
pt.set_editor_property("pin_category", "object")
pt.set_editor_property("pin_sub_category_object", unreal.SoundBase.static_class())
ok = unreal.BlueprintEditorLibrary.add_member_variable(bp, "PickupSound", pt)
unreal.log(MARK + ": add PickupSound -> " + str(ok))
unreal.BlueprintEditorLibrary.set_blueprint_variable_instance_editable(bp, "PickupSound", True)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)

cls = unreal.load_object(None, BP_PATH + ".BP_LootPickup_C")
cdo = unreal.get_default_object(cls)
sound = unreal.load_asset("/Game/Audio/buff-1")
cdo.set_editor_property("PickupSound", sound)
unreal.log(MARK + ": CDO PickupSound = " + str(cdo.get_editor_property("PickupSound")))
unreal.log(MARK + ": CDO bIsGold = " + str(cdo.get_editor_property("bIsGold")))
unreal.log(MARK + ": CDO GoldAmount = " + str(cdo.get_editor_property("GoldAmount")))

saved = unreal.EditorAssetLibrary.save_asset(BP_PATH)
unreal.log(MARK + ": save_asset -> " + str(saved))
