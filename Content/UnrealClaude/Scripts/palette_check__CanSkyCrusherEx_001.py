import unreal

# Is the UFunction actually in the loaded module (i.e. would it show in the BP palette)?
lib = unreal.NexusAbilityUILibrary
names = [n for n in dir(lib) if "sky" in n.lower() or "crusher" in n.lower()]
print("PALETTE_CHECK sky/crusher members on NexusAbilityUILibrary:", names)
print("HAS_can_sky_crusher_execute:", hasattr(lib, "can_sky_crusher_execute"))

# Reflection-level confirmation: does the UClass expose the UFunction by its native name?
cdo = unreal.get_default_object(unreal.NexusAbilityUILibrary)
print("CDO:", cdo)

# Confirm the GA blueprint's generated class + parent
ga = unreal.EditorAssetLibrary.load_asset("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher")
print("GA:", ga, "parent:", ga.get_editor_property("parent_class"))
