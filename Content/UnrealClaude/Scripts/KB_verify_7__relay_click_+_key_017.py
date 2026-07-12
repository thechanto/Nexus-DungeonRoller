import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
screen = unreal.find_object(None, "/Engine/Transient.UnrealEdEngine_0:BP_MyGameInstance_C_1.W_AbilitiesScreen_C_0")
lib = unreal.NexusAbilityUILibrary
ctrl = unreal.find_object(None, "/Engine/Transient.NexusAbilityPreviewController_0")
target = None
for i in range(0, 40):
    r = unreal.find_object(None, "/Engine/Transient.NexusAbilityPreviewController_0.NexusAbilitySlotRelay_%d" % i)
    if not r: continue
    r.call_method("HandleSlotClicked", args=(None,))
    sel = lib.get_selected_ability(screen)
    if sel and sel.get_name() == "DA_Ability_Warrior_LeapSlash_C":
        target = r
        break
print("KB7 selected via relay=" + str(lib.get_selected_ability(screen)))
def colors():
    out = []
    for i in range(1, 5):
        b = screen.get_editor_property("Keybind" + str(i))
        c = b.background_color
        out.append("K%d en=%s (%.2f,%.2f,%.2f)" % (i, b.get_is_enabled(), c.r, c.g, c.b))
    return " | ".join(out)
print("KB7 colors after select: " + colors())
print("KB7 before assigned=" + str([s.split(".")[-1] for s in lib.get_assigned_abilities(world)]))
ctrl.call_method("HandleKeybind2Clicked")
print("KB7 after K2 assigned=" + str([s.split(".")[-1] for s in lib.get_assigned_abilities(world)]))
print("KB7 colors after K2: " + colors())
ctrl.call_method("HandleKeybind2Clicked")
print("KB7 after toggle assigned=" + str([s.split(".")[-1] for s in lib.get_assigned_abilities(world)]))
print("KB7 colors after toggle: " + colors())
ctrl.call_method("HandleKeybind1Clicked")
print("KB7 after K1 assigned=" + str([s.split(".")[-1] for s in lib.get_assigned_abilities(world)]))
print("KB7 colors after K1: " + colors())
print("KB7 DONE")