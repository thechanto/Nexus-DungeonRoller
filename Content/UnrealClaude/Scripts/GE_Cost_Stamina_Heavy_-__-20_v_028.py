import unreal, re
libcdo = unreal.get_default_object(unreal.NexusAbilityUILibrary)
FX = "/Game/GameplayAbilitySystem/Effects/"
DSTN = "GE_Cost_Stamina_Heavy"
dst = FX + DSTN
cls = unreal.load_object(None, dst + "." + DSTN + "_C")
txt = str(libcdo.call_method("GetClassDefaultPropertyText", (cls, "Modifiers")))
new_txt, n = re.subn(r'(ScalableFloatMagnitude=\(Value=)-10\.000000', r'\g<1>-20.000000', txt)
if n != 1:
    raise RuntimeError(f"expected 1 value replacement, got {n}; text head: {txt[:300]}")
if not libcdo.call_method("SetClassDefaultPropertyText", (cls, "Modifiers", new_txt)):
    raise RuntimeError("ImportText failed")
chk = str(libcdo.call_method("GetClassDefaultPropertyText", (cls, "Modifiers")))
if "Value=-20.000000" not in chk or "DecreaseBy20Percent" not in chk:
    raise RuntimeError(f"verify failed: {chk[:300]}")
saved = unreal.EditorAssetLibrary.save_asset(dst, only_if_is_dirty=False)
print(f"[STEP-OK] {DSTN}: Stamina -20 (curve kept) saved={saved}")
