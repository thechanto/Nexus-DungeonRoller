import unreal
GA = "/Game/GameplayAbilitySystem/Abilities/GA_MeleeAttack_AxeSwing"
GE = "/Game/GameplayAbilitySystem/Effects/GE_Cost_Stamina_Heavy"
gec = unreal.load_object(None, GE + "." + GE.split("/")[-1] + "_C")
cls = unreal.load_object(None, GA + "." + GA.split("/")[-1] + "_C")
if not gec or not cls:
    raise RuntimeError(f"load failed: gec={gec} cls={cls}")
cdo = unreal.get_default_object(cls)
cdo.set_editor_property("cost_gameplay_effect_class", gec)
v = cdo.get_editor_property("cost_gameplay_effect_class")
if not v or v.get_name() != "GE_Cost_Stamina_Heavy_C":
    raise RuntimeError(f"verify failed: {v}")
saved = unreal.EditorAssetLibrary.save_asset(GA, only_if_is_dirty=False)
print(f"[STEP-OK] GA_MeleeAttack_AxeSwing CostGE={v.get_name()} saved={saved}")
