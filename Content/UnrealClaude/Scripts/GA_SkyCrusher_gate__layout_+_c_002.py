import unreal

BP_PATH = "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher"
bp = unreal.EditorAssetLibrary.load_asset(BP_PATH)

# --- lay the new nodes out so the graph is readable (add_node ignored 'position') ---
LAYOUT = {
    "Can Sky Crusher Execute": (330, -190),
    "Branch":                  (640, -190),
    "End Ability":             (900, -60),
}
try:
    pages = bp.get_editor_property("ubergraph_pages")
    for g in pages:
        for n in g.get_editor_property("nodes"):
            title = n.get_node_title(unreal.NodeTitleType.LIST_VIEW) if hasattr(n, "get_node_title") else ""
            title = str(title).strip()
            for key, (x, y) in LAYOUT.items():
                if key.lower() in title.lower():
                    n.set_editor_property("node_pos_x", x)
                    n.set_editor_property("node_pos_y", y)
                    print("LAYOUT set", title, "->", x, y)
except Exception as e:
    print("LAYOUT skipped (cosmetic only):", e)

# --- compile + save ---
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
print("COMPILE_STATUS:", bp.get_editor_property("status") if bp else "?")
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
print("SAVED:", saved)

# --- export T3D straight from the saved asset for independent verification ---
out_dir = unreal.Paths.project_dir() + "T3D_Verify/"
task = unreal.AssetExportTask()
task.set_editor_property("object", bp)
task.set_editor_property("filename", out_dir + "GA_SkyCrusher.T3D")
task.set_editor_property("automated", True)
task.set_editor_property("prompt", False)
task.set_editor_property("exporter", unreal.Exporter.get_default_object())
ok = unreal.Exporter.run_asset_export_task(task)
print("T3D_EXPORT:", ok, out_dir + "GA_SkyCrusher.T3D")
