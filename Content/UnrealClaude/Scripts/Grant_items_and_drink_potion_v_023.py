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

def cls_of(name):
    return unreal.load_class(None, f"/Game/Inventory/Items/{name}.{name}_C")

unreal.log(f"{M} flask_before={flask_count()}")
for name, qty in [("BP_Item_Gold", 250), ("BP_Item_HealthPotion", 3),
                  ("BP_Item_Weapon_RustedAxe", 1), ("BP_Item_Weapon_ApprenticeStaff", 1)]:
    c = cls_of(name)
    res = run_inv.try_add_item_from_class(c, qty)
    unreal.log(f"{M} add {name} x{qty} given={res.get_editor_property('amount_given')}")

for it in run_inv.get_items():
    unreal.log(f"{M} inv: {it.get_class().get_name()} x{it.get_editor_property('quantity')}")

pot = run_inv.find_item_by_class(cls_of("BP_Item_HealthPotion"))
run_inv.use_item(pot)
pot2 = run_inv.find_item_by_class(cls_of("BP_Item_HealthPotion"))
q2 = pot2.get_editor_property("quantity") if pot2 else 0
unreal.log(f"{M} after_drink: flask={flask_count()} potion_stack={q2}")
