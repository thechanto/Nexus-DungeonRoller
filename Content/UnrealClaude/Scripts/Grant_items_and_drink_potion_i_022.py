import unreal
M = "[ITEMTEST]"
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
ps = unreal.GameplayStatics.get_player_state(w, 0)
run_inv = next(c for c in ps.get_components_by_class(unreal.NarrativeInventoryComponent) if c.get_name() == "RunInventory")

def flask_count():
    for n in ("HealthPotionCount ", "HealthPotionCount"):
        try:
            return pawn.get_editor_property(n)
        except Exception:
            pass
    return None

unreal.log(f"{M} flask_before={flask_count()}")

items = [("BP_Item_Gold", 250), ("BP_Item_HealthPotion", 3),
         ("BP_Item_Weapon_RustedAxe", 1), ("BP_Item_Weapon_ApprenticeStaff", 1)]
for name, qty in items:
    cls = unreal.EditorAssetLibrary.load_blueprint_class(f"/Game/Inventory/Items/{name}")
    try:
        res = run_inv.try_add_item_from_class(cls, qty)
        unreal.log(f"{M} add {name} x{qty} -> {res}")
    except Exception as e:
        unreal.log(f"{M} add {name} FAILED: {e}")

for it in run_inv.get_items():
    try:
        q = it.get_editor_property("Quantity")
    except Exception:
        q = "?"
    unreal.log(f"{M} inv: {it.get_class().get_name()} x{q}")

pot_cls = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Inventory/Items/BP_Item_HealthPotion")
pot = run_inv.find_item_by_class(pot_cls)
unreal.log(f"{M} potion_instance={pot}")
try:
    run_inv.use_item(pot)
    unreal.log(f"{M} use_item called")
except Exception as e:
    unreal.log(f"{M} use_item FAILED: {e}")

pot2 = run_inv.find_item_by_class(pot_cls)
q2 = pot2.get_editor_property("Quantity") if pot2 else 0
unreal.log(f"{M} flask_after={flask_count()} potion_stack_after={q2}")
