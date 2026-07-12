import unreal

out = unreal.Paths.project_dir() + "_ai/t3d"
at = unreal.AssetToolsHelpers.get_asset_tools()

# --- 1) LevelScriptBlueprint of LV_Soul_Cave ---------------------------------
for cand in [
    "/Game/SoulCave/Maps/LV_Soul_Cave.LV_Soul_Cave:PersistentLevel.LV_Soul_Cave",
    "/Game/SoulCave/Maps/LV_Soul_Cave.LV_Soul_Cave",
]:
    o = unreal.load_object(None, cand)
    unreal.log("CAND %s -> %s" % (cand, type(o).__name__ if o else "None"))

w = unreal.load_object(None, "/Game/SoulCave/Maps/LV_Soul_Cave.LV_Soul_Cave")
lvl = w.get_editor_property("persistent_level") if w else None
unreal.log("LEVEL=%s" % lvl)
lsb = lvl.get_editor_property("level_script_blueprint") if lvl else None
unreal.log("LSB=%s path=%s" % (lsb, lsb.get_path_name() if lsb else "None"))

if lsb:
    at.export_assets([lsb.get_path_name()], out)
    unreal.log("EXPORTED_LSB")

# --- 2) AM_SkyCrusher ---------------------------------------------------------
am = unreal.load_object(None, "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher.AM_SkyCrusher")
if am:
    unreal.log("MONTAGE len=%s rate=%s" % (
        am.get_editor_property("sequence_length"),
        am.get_editor_property("rate_scale")))
    at.export_assets(["/Game/RetargetedAbilities/Warrior/AM_SkyCrusher.AM_SkyCrusher"], out)

# --- 3) Enemy class taxonomy --------------------------------------------------
for p in [
    "/Game/Enemies/BP_Enemy_Base.BP_Enemy_Base_C",
    "/Game/Enemies/MeleeEnemy/BP_Enemy_Melee.BP_Enemy_Melee_C",
    "/Game/Enemies/BP_Enemy_Ranged.BP_Enemy_Ranged_C",
    "/Game/Enemies/MageEnemy/BP_Enemy_Mage.BP_Enemy_Mage_C",
    "/Game/Enemies/BossEnemy/BP_Enemy_Boss.BP_Enemy_Boss_C",
]:
    c = unreal.load_class(None, p)
    unreal.log("CLASS %s -> %s" % (p, c.get_super_class().get_name() if c else "None"))

# --- 4) Existing camera shake ------------------------------------------------
at.export_assets(["/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy.BP_CameraShake_Hit_Enemy"], out)
unreal.log("DONE")
