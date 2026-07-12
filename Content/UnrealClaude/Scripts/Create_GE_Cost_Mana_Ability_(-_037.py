import unreal, re
libcdo = unreal.get_default_object(unreal.NexusAbilityUILibrary)
FX = "/Game/GameplayAbilitySystem/Effects/"
dst = FX + "GE_Cost_Mana_Ability"
eal = unreal.EditorAssetLibrary
if not eal.does_asset_exist(dst):
    if not eal.duplicate_asset(FX + "GE_Cost_Mana", dst):
        raise RuntimeError("duplicate failed")
cls = unreal.load_object(None, dst + ".GE_Cost_Mana_Ability_C")
txt = str(libcdo.call_method("GetClassDefaultPropertyText", (cls, "Modifiers")))
new_txt, n = re.subn(r'(ScalableFloatMagnitude=\(Value=)-10\.000000', r'\g<1>-33.000000', txt)
if n != 1:
    raise RuntimeError(f"expected 1 replacement, got {n}; head: {txt[:300]}")
if not libcdo.call_method("SetClassDefaultPropertyText", (cls, "Modifiers", new_txt)):
    raise RuntimeError("ImportText failed")
chk = str(libcdo.call_method("GetClassDefaultPropertyText", (cls, "Modifiers")))
if "Value=-33.000000" not in chk or "DecreaseBy20Percent" not in chk or 'AttributeName="Mana"' not in chk:
    raise RuntimeError(f"verify failed: {chk[:300]}")
saved = eal.save_asset(dst, only_if_is_dirty=False)
print(f"[STEP-OK] GE_Cost_Mana_Ability: Mana -33.000000 (curve kept) saved={saved}")
