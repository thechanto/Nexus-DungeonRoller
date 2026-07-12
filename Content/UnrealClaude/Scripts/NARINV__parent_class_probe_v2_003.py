import unreal

paths = [
    "/NarrativeInventory/NarrativeUI/WBP_InventoryWidget.WBP_InventoryWidget",
    "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory",
    "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting",
    "/NarrativeInventory/NarrativeUI/WBP_Item.WBP_Item",
    "/NarrativeInventory/Blueprints/BP_BasicItemPickup.BP_BasicItemPickup",
]
print("=== PARENT (super) CLASSES ===")
for p in paths:
    n = p.split(".")[-1]
    try:
        bp = unreal.load_asset(p)
        if not bp:
            print("%-28s -> LOAD FAILED" % n)
            continue
        gc = bp.generated_class()
        sup = gc.get_super_class() if gc else None
        print("%-28s -> %s" % (n, sup.get_path_name() if sup else "None"))
    except Exception as e:
        print("%-28s -> ERR %s" % (n, e))

print("")
print("=== CLASS RESOLUTION (proves module loaded) ===")
checks = [
    "/Script/CommonUI.CommonActivatableWidget",
    "/Script/CommonUI.CommonButtonBase",
    "/Script/NarrativeInventory.NarrativeInventoryComponent",
    "/Script/NarrativeInventory.NarrativeItem",
    "/Script/NarrativeEquipment.EquipmentComponent",
    "/Script/NarrativeEquipment.EquippableItem",
]
for c in checks:
    try:
        o = unreal.load_object(None, c)
        print("  %-56s %s" % (c, "OK" if o else "MISSING"))
    except Exception as e:
        print("  %-56s MISSING (%s)" % (c, type(e).__name__))

print("")
print("=== InventoryComponent: does it need a Pawn owner? ===")
cdo = unreal.get_default_object(unreal.NarrativeInventoryComponent)
for prop in ["currency", "capacity", "max_weight", "default_items"]:
    try:
        print("  %-16s = %s" % (prop, cdo.get_editor_property(prop)))
    except Exception as e:
        print("  %-16s <not exposed>" % prop)
