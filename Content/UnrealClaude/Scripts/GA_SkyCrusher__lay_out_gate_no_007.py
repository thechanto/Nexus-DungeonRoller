import unreal

BP_PATH = "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher"
GRAPH = "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher.GA_SkyCrusher:Gameplay Ability Graph."

# node names come from the T3D export
LAYOUT = {
    "K2Node_CallFunction_0": (300,  -180),   # Can Sky Crusher Execute
    "K2Node_IfThenElse_0":   (660,  -180),   # Branch
    "K2Node_CallFunction_1": (940,   -30),   # End Ability  (false)
    "K2Node_CallParentFunction_0": (940, -180),  # Parent: ActivateAbility (true)
}
for name, (x, y) in LAYOUT.items():
    n = unreal.load_object(None, GRAPH + name)
    if not n:
        print("MISSING:", name); continue
    n.set_editor_property("node_pos_x", x)
    n.set_editor_property("node_pos_y", y)
    print("POS", name, "->", x, y)

bp = unreal.EditorAssetLibrary.load_asset(BP_PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
print("SAVED:", unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False))
