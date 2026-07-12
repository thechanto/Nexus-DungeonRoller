# @Description palette check: CanSkyCrusherExecute present in running editor reflection
import unreal
print("=== PALETTE CHECK ===")
try:
    uclass = unreal.load_object(None, "/Script/Nexus.NexusAbilityUILibrary")
    print("UCLASS_LOADED:", uclass is not None)
except Exception as e:
    print("UCLASS_LOAD_FAIL:", e)
lib = getattr(unreal, "NexusAbilityUILibrary", None)
print("PY_WRAPPER_EXISTS:", lib is not None)
if lib:
    hits = sorted([n for n in dir(lib) if ("sky" in n.lower() or "crusher" in n.lower())])
    print("SKY_METHODS:", hits)
    print("HAS_CAN_SKY_CRUSHER_EXECUTE:", hasattr(lib, "can_sky_crusher_execute"))
    print("SANITY_HAS_SPAWN_LOOT_DROP:", hasattr(lib, "spawn_loot_drop"))
    print("SANITY_HAS_ASSIGN_KEYBIND:", hasattr(lib, "assign_ability_to_keybind_slot"))
print("=== END ===")