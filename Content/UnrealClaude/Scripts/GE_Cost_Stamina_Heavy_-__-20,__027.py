import unreal, re
lib = unreal.NexusAbilityUILibrary
if not hasattr(lib, "get_class_default_property_text"):
    raise RuntimeError("helper UFUNCTIONs not visible to python - editor restart + rebuild needed")
FX = "/Game/GameplayAbilitySystem/Effects/"
DSTN = "GE_Cost_Stamina_Heavy"
dst = FX + DSTN
cls = unreal.load_object(None, dst + "." + DSTN + "_C")
txt = str(lib.get_class_default_property_text(cls, "Modifiers"))
new_txt, n = re.subn(r'(ScalableFloatMagnitude=\(Value=)-10\.000000', r'\g<1>-20.000000', txt)
if n != 1:
    raise RuntimeError(f"expected 1 value replacement, got {n}; text head: {txt[:300]}")
if not lib.set_class_default_property_text(cls, "Modifiers", new_txt):
    raise RuntimeError("ImportText failed")
chk = str(lib.get_class_default_property_text(cls, "Modifiers"))
if "Value=-20.000000" not in chk or "DecreaseBy20Percent" not in chk:
    raise RuntimeError(f"verify failed: {chk[:300]}")
saved = unreal.EditorAssetLibrary.save_asset(dst, only_if_is_dirty=False)
print(f"[STEP-OK] {DSTN}: Stamina -20 (curve DecreaseBy20Percent kept) saved={saved}")
