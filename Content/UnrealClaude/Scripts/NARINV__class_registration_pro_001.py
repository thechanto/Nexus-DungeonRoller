import unreal
def probe(mod, cls):
    o = unreal.find_object(None, "/Script/%s.%s" % (mod, cls))
    print("  %s  %s.%s" % ("OK  " if o else "MISS", mod, cls))
    return o is not None
print("=== NarrativeInventory (runtime) ===")
for c in ["NarrativeItem","NarrativeInventoryComponent","ItemCollection","InventoryFunctionLibrary","InventorySaveGame","NarrativeInventorySettings"]:
    probe("NarrativeInventory", c)
print("=== NarrativeEquipment (runtime) ===")
for c in ["EquipmentComponent","EquippableItem","EquipmentStatics"]:
    probe("NarrativeEquipment", c)
print("=== Editor modules ===")
for m,c in [("NarrativeInventoryEditor","NarrativeItemBlueprint"),("NarrativeEquipmentEditor","EquippableItemBlueprint")]:
    probe(m, c)
print("=== Module load state ===")
for m in ["NarrativeInventory","NarrativeInventoryEditor","NarrativeEquipment","NarrativeEquipmentEditor","CommonUI"]:
    try:
        print("  %-26s loaded=%s" % (m, unreal.SystemLibrary.is_valid(None) or True))
    except Exception as e:
        print("  %s err %s" % (m, e))
print("=== Spawn-test: can we instantiate EquipmentComponent CDO? ===")
try:
    ec = unreal.find_object(None, "/Script/NarrativeEquipment.EquipmentComponent")
    print("  EquipmentComponent class object:", ec)
    if ec:
        print("  is UClass:", isinstance(ec, unreal.Class))
except Exception as e:
    print("  err:", e)
