import unreal
G = "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher.GA_SkyCrusher:Gameplay Ability Graph."
LAYOUT = {
    "K2Node_Event_0":              (-40,  -180),
    "K2Node_CallFunction_0":       (300,  -180),   # Can Sky Crusher Execute
    "K2Node_IfThenElse_0":         (680,  -180),   # Branch
    "K2Node_CallParentFunction_0": (960,  -200),   # true  -> Parent: ActivateAbility
    "K2Node_CallFunction_1":       (960,   -40),   # false -> End Ability
}
for name, (x, y) in LAYOUT.items():
    n = unreal.load_object(None, G + name)
    unreal.log_warning("LAYOUT {} -> {} {} (obj={})".format(name, x, y, n))
    if n:
        n.set_editor_property("node_pos_x", x)
        n.set_editor_property("node_pos_y", y)
bp = unreal.EditorAssetLibrary.load_asset("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher")
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.log_warning("LAYOUT_SAVED {}".format(unreal.EditorAssetLibrary.save_asset("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher", only_if_is_dirty=False)))
