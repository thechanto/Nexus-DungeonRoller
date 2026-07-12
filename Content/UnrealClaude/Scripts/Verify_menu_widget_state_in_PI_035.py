import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
widgets = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C"), False)
print("[VERIFY] menu instances:", len(widgets))
for w in widgets:
    print("[VERIFY] menu:", w.get_name(), "activated=", w.call_method("BP_GetDesiredFocusTarget") is not None)
    print("[VERIFY] activated=", w.is_activated(), "visible=", w.get_visibility())
# what inventory does the embedded InventoryWidget show?
inner = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, unreal.load_class(None, "/NarrativeInventory/NarrativeUI/WBP_InventoryWidget.WBP_InventoryWidget_C"), False)
print("[VERIFY] inventory widgets:", len(inner))
for iw in inner:
    try:
        print("[VERIFY] inv widget:", iw.get_name(), "vis=", iw.get_visibility())
        for pname in ["InventoryComponent", "Inventory", "OwnerInventory"]:
            try:
                v = iw.get_editor_property(pname)
                print("[VERIFY]   prop", pname, "=", v.get_path_name() if v else None)
            except Exception:
                pass
    except Exception as e:
        print("[VERIFY] inner err:", e)
# W_PlayerHUD still up?
hud = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, unreal.load_class(None, "/Game/Widgets/W_PlayerHUD.W_PlayerHUD_C"), False)
print("[VERIFY] W_PlayerHUD instances:", len(hud), "in_viewport=", hud[0].is_in_viewport() if hud else None)
# input mode
pc = unreal.GameplayStatics.get_player_controller(world, 0)
print("[VERIFY] show_mouse_cursor =", pc.get_editor_property("bShowMouseCursor"))
