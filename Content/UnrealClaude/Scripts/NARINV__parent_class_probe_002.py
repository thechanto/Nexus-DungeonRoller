import unreal

paths = [
    "/NarrativeInventory/NarrativeUI/WBP_InventoryWidget.WBP_InventoryWidget",
    "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory",
    "/NarrativeInventory/NarrativeUI/Looting/W_NarrativeMenu_Looting.W_NarrativeMenu_Looting",
    "/NarrativeInventory/NarrativeUI/WBP_Item.WBP_Item",
    "/NarrativeInventory/Blueprints/BP_BasicItemPickup.BP_BasicItemPickup",
]
print("=== PARENT CLASSES ===")
for p in paths:
    n = p.split(".")[-1]
    try:
        bp = unreal.load_asset(p)
        if not bp:
            print("%-30s -> LOAD FAILED" % n)
            continue
        pc = bp.get_editor_property("parent_class")
        print("%-30s -> %s" % (n, pc.get_path_name() if pc else "None"))
    except Exception as e:
        print("%-30s -> ERR %s" % (n, e))

print("")
print("=== DEPENDENCY MODULES PRESENT? ===")
for m in ["CommonUI", "NarrativeArsenal", "NarrativeNavigation", "NarrativeCommon", "NarrativeTales", "NarrativeQuests"]:
    try:
        o = unreal.load_object(None, "/Script/" + m)
        print("  %-22s %s" % (m, "LOADED" if o else "absent"))
    except Exception:
        print("  %-22s absent" % m)

print("")
print("=== KEY CLASS RESOLUTION ===")
for c in ["/Script/CommonUI.CommonActivatableWidget",
          "/Script/NarrativeInventory.NarrativeInventoryComponent",
          "/Script/NarrativeEquipment.EquipmentComponent"]:
    try:
        o = unreal.load_object(None, c)
        print("  %-58s %s" % (c, "OK" if o else "MISSING"))
    except Exception as e:
        print("  %-58s ERR %s" % (c, e))
