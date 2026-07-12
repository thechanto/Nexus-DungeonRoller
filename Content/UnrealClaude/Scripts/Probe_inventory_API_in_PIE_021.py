import unreal
M = "[ITEMTEST]"
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
unreal.log(f"{M} world={w.get_name() if w else None}")
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
ps = unreal.GameplayStatics.get_player_state(w, 0)
unreal.log(f"{M} pawn={pawn.get_name() if pawn else None} ps={ps.get_name() if ps else None}")
invclass = getattr(unreal, "NarrativeInventoryComponent", None)
unreal.log(f"{M} invclass={invclass}")
comps = ps.get_components_by_class(invclass)
for c in comps:
    unreal.log(f"{M} comp={c.get_name()}")
run_inv = next((c for c in comps if c.get_name() == "RunInventory"), None)
cands = [n for n in dir(run_inv) if any(k in n.lower() for k in ("add", "give", "use", "item", "stack"))]
unreal.log(f"{M} api={cands}")
# item-side API too
item_cls = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Inventory/Items/BP_Item_HealthPotion")
cdo = unreal.get_default_object(item_cls)
icands = [n for n in dir(cdo) if any(k in n.lower() for k in ("use", "consume", "owner"))]
unreal.log(f"{M} itemapi={icands}")
