import unreal, re
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if not world:
    raise RuntimeError("no PIE world")
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Character):
    if "NexusPlayer" in p.get_name():
        player = p
if not player:
    raise RuntimeError("player pawn not found")

cls = unreal.load_object(None, "/Game/GameplayAbilitySystem/Effects/GE_SetStats.GE_SetStats_C")
cdo = unreal.get_default_object(cls)
attrs = {}
for m in cdo.get_editor_property("modifiers"):
    a = m.get_editor_property("attribute")
    name = re.search(r'AttributeName="(\w+)"', a.export_text())
    if name:
        attrs[name.group(1)] = a

lib = unreal.AbilitySystemBlueprintLibrary
fn = getattr(lib, "get_float_attribute", None) or getattr(lib, "get_float_attribute_base", None)
out = []
for name in ["MaxHealth", "MaxStamina", "MaxMana"]:
    r = fn(player, attrs[name])
    val = r[0] if isinstance(r, tuple) else r
    out.append(f"{name}={val}")
print("[PIE-CHECK] " + ", ".join(str(o) for o in out))
