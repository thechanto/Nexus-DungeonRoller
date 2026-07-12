import unreal

BP_PATH = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController"
bp = unreal.load_asset(BP_PATH)
print("[COMP] bp =", bp)

comp_class = unreal.load_class(None, "/Script/Nexus.NexusInventoryUIComponent")
print("[COMP] class =", comp_class)

sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sds.k2_gather_subobject_data_for_blueprint(bp)
print("[COMP] existing handles:", len(handles))

# check for an existing component of this class (idempotent)
existing = None
for h in handles:
    data = sds.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if obj and obj.get_class().get_name() == "NexusInventoryUIComponent":
        existing = obj
        print("[COMP] already present:", obj.get_path_name())
        break

if existing is None:
    params = unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=comp_class, blueprint_context=bp)
    new_handle, fail = sds.add_new_subobject(params)
    print("[COMP] add result fail_reason='%s'" % fail)
    sds.rename_subobject(new_handle, "InventoryUI")
    data = sds.k2_find_subobject_data_from_handle(new_handle)
    existing = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
print("[COMP] template obj =", existing.get_path_name())

hud_cls = unreal.load_class(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD_C")
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C")
ia = unreal.load_asset("/Game/Input/Actions/IA_Inventory")
print("[COMP] hud_cls=%s menu_cls=%s ia=%s" % (hud_cls, menu_cls, ia))

existing.set_editor_property("HUDWidgetClass", hud_cls)
existing.set_editor_property("InventoryMenuClass", menu_cls)
existing.set_editor_property("ToggleAction", ia)

# verify by re-reading
print("[COMP] verify HUDWidgetClass =", existing.get_editor_property("HUDWidgetClass"))
print("[COMP] verify InventoryMenuClass =", existing.get_editor_property("InventoryMenuClass"))
print("[COMP] verify ToggleAction =", existing.get_editor_property("ToggleAction"))

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
print("[COMP] saved:", unreal.EditorAssetLibrary.save_asset(BP_PATH))
