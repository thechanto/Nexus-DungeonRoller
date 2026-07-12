import unreal, os

L = []
def p(s):
    L.append(str(s))
    unreal.log("A2| " + str(s))

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
    p("EXPORT %s -> %s.T3D ok=%s" % (pkg, nice, ok))

# parent chain of GA_SkyCrusher, read off the Blueprint asset
bp = unreal.EditorAssetLibrary.load_asset("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher")
p("GA_SkyCrusher parent_class = %s" % bp.get_editor_property("parent_class"))

ar = unreal.AssetRegistryHelpers.get_asset_registry()
wanted = ("GA_MeleeAttack_Base", "AN_HitScanStart", "AN_HitScanEnd", "GA_Death", "DamageCalculationModifier")
for a in ar.get_assets_by_path("/Game", recursive=True):
    n = str(a.asset_name)
    if n in wanted:
        p("FOUND %s at %s" % (n, a.package_name))
        export(str(a.package_name), n)

with open("C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/a2_out.txt", "w") as f:
    f.write("\n".join(L))
p("DONE")
