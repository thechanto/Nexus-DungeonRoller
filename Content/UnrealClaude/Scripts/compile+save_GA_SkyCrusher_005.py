# @Description compile + save GA_SkyCrusher, report dirty/compile status
import unreal
p = "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher"
bp = unreal.EditorAssetLibrary.load_asset(p)
print("LOADED:", bp)
pkg = bp.get_outer()
try:
    print("DIRTY_BEFORE:", pkg.is_dirty())
except Exception as e:
    print("DIRTY_BEFORE: n/a", e)
try:
    st = bp.get_editor_property("status")
    print("STATUS_BEFORE:", st)
except Exception as e:
    print("STATUS_BEFORE: n/a", e)
print("--- compiling ---")
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
try:
    st2 = bp.get_editor_property("status")
    print("STATUS_AFTER:", st2)
except Exception as e:
    print("STATUS_AFTER: n/a", e)
saved = unreal.EditorAssetLibrary.save_asset(p, only_if_is_dirty=False)
print("SAVE_ASSET_RETURNED:", saved)
try:
    print("DIRTY_AFTER_SAVE:", pkg.is_dirty())
except Exception as e:
    print("DIRTY_AFTER_SAVE: n/a", e)
print("=== DONE ===")