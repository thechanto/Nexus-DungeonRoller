import unreal, os

L = []
def p(s):
    L.append(str(s))
    unreal.log("APATH| " + str(s))

lib = unreal.NexusAbilityUILibrary

# ---- GE_Damage_Instant: modifiers + SetByCaller tag ------------------------
ge = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Effects/GE_Damage_Instant")
p("GE_CLASS=%s" % ge)
for prop in ["Modifiers", "DurationPolicy", "InheritableGameplayEffectTags", "InheritableOwnedTagsContainer"]:
    try:
        txt = lib.get_class_default_property_text(ge, prop)
        p("GE.%s = %s" % (prop, txt))
    except Exception as e:
        p("GE.%s ERR %s" % (prop, e))

# ---- GA_SkyCrusher parent + GA_MeleeAttack_Base graph ----------------------
gasc = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher")
p("GA_SkyCrusher class = %s" % gasc)
p("GA_SkyCrusher parent = %s" % gasc.get_super_class())

out_dir = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/T3D_Verify"
os.makedirs(out_dir, exist_ok=True)

def export(pkg, nice):
    a = unreal.EditorAssetLibrary.load_asset(pkg)
    if not a:
        p("EXPORT %s -> NOT FOUND" % pkg)
        return
    t = unreal.AssetExportTask()
    t.set_editor_property("automated", True)
    t.set_editor_property("object", a)
    t.set_editor_property("filename", "%s/%s.T3D" % (out_dir, nice))
    t.set_editor_property("prompt", False)
    t.set_editor_property("replace_identical", True)
    t.set_editor_property("write_empty_files", False)
    ok = unreal.Exporter.run_asset_export_task(t)
    p("EXPORT %s -> %s ok=%s" % (pkg, nice, ok))

# find the melee base + the notify BPs
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for a in ar.get_assets_by_path("/Game", recursive=True):
    n = str(a.asset_name)
    if n in ("GA_MeleeAttack_Base", "AN_HitScanStart", "AN_HitScanEnd", "GA_Death"):
        p("FOUND %s at %s" % (n, a.package_name))
        export(str(a.package_name), n)

with open("C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/a1_out.txt", "w") as f:
    f.write("\n".join(L))
p("DONE")
