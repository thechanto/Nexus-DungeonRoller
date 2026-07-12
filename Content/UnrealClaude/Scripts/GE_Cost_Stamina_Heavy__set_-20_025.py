import unreal
FX = "/Game/GameplayAbilitySystem/Effects/"
DSTN = "GE_Cost_Stamina_Heavy"
NEWVAL = -20.0
dst = FX + DSTN
eal = unreal.EditorAssetLibrary
cls = unreal.load_object(None, dst + "." + DSTN + "_C")
cdo = unreal.get_default_object(cls)
mods = cdo.get_editor_property("modifiers")
m = mods[0]
attr = m.get_editor_property("attribute")
op = m.get_editor_property("modifier_op")
mag = m.get_editor_property("modifier_magnitude")
sf = mag.get_editor_property("scalable_float_magnitude")
sf.set_editor_property("value", NEWVAL)
newmag = unreal.GameplayEffectModifierMagnitude(
    magnitude_calculation_type=unreal.GameplayEffectMagnitudeCalculation.SCALABLE_FLOAT,
    scalable_float_magnitude=sf)
newmod = unreal.GameplayModifierInfo(attribute=attr, modifier_op=op, modifier_magnitude=newmag)
mods[0] = newmod
cdo.set_editor_property("modifiers", mods)
chk = cdo.get_editor_property("modifiers")[0]
cm = chk.get_editor_property("modifier_magnitude").get_editor_property("scalable_float_magnitude")
v = cm.get_editor_property("value")
curve = cm.get_editor_property("curve").export_text()
if abs(v - NEWVAL) > 0.001:
    raise RuntimeError(f"verify failed: value={v}")
ok = eal.save_asset(dst, only_if_is_dirty=False)
print(f"[STEP-OK] {DSTN}: value={v} curve={curve} saved={ok}")
