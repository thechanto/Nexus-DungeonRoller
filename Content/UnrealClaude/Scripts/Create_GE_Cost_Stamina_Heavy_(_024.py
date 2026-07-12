import unreal
FX = "/Game/GameplayAbilitySystem/Effects/"
SRC = FX + "GE_Cost_Stamina"
DSTN = "GE_Cost_Stamina_Heavy"
NEWVAL = -20.0
eal = unreal.EditorAssetLibrary
dst = FX + DSTN
if eal.does_asset_exist(dst):
    print(f"[STEP] {DSTN} already exists - editing in place")
else:
    dup = eal.duplicate_asset(SRC, dst)
    print(f"[STEP] duplicated {SRC.split('/')[-1]} -> {dup}")
cls = unreal.load_object(None, dst + "." + DSTN + "_C")
cdo = unreal.get_default_object(cls)
mods = cdo.get_editor_property("modifiers")
m = mods[0]
mag = m.get_editor_property("modifier_magnitude")
sf = mag.get_editor_property("scalable_float_magnitude")
sf.set_editor_property("value", NEWVAL)
mag.set_editor_property("scalable_float_magnitude", sf)
m.set_editor_property("modifier_magnitude", mag)
mods[0] = m
cdo.set_editor_property("modifiers", mods)
chk = cdo.get_editor_property("modifiers")[0]
v = chk.get_editor_property("modifier_magnitude").get_editor_property("scalable_float_magnitude").get_editor_property("value")
attr = chk.get_editor_property("attribute")
ok = eal.save_asset(dst, only_if_is_dirty=False)
print(f"[STEP-OK] {DSTN}: attr={attr.export_text()[:80]} value={v} saved={ok}")
