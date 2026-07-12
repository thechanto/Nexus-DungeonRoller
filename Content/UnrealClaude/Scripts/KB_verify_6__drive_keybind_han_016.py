import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
screen = unreal.find_object(None, "/Engine/Transient.UnrealEdEngine_0:BP_MyGameInstance_C_1.W_AbilitiesScreen_C_0")
lib = unreal.NexusAbilityUILibrary
ctrl = None
for i in range(0, 30):
    c = unreal.find_object(None, "/Engine/Transient.NexusAbilityPreviewController_%d" % i)
    if c: ctrl = c
print("KB6 controller=" + str(ctrl))
data = screen.get_editor_property("WarriorSlot_LeapSlash").get_editor_property("AbilityData")
print("KB6 before assigned=" + str(list(lib.get_assigned_abilities(world))))
ctrl.set_editor_property("SelectedAbility", data)
ctrl.call_method("HandleKeybind2Clicked")
print("KB6 after K2 assigned=" + str(list(lib.get_assigned_abilities(world))))
def colors():
    out = []
    for i in range(1, 5):
        b = screen.get_editor_property("Keybind" + str(i))
        c = b.background_color
        out.append("K%d en=%s (%.2f,%.2f,%.2f)" % (i, b.get_is_enabled(), c.r, c.g, c.b))
    return " | ".join(out)
print("KB6 colors after K2: " + colors())
ctrl.call_method("HandleKeybind2Clicked")
print("KB6 after K2 toggle assigned=" + str(list(lib.get_assigned_abilities(world))))
print("KB6 colors after toggle: " + colors())
ctrl.call_method("HandleKeybind1Clicked")
print("KB6 after K1 assigned=" + str(list(lib.get_assigned_abilities(world))))
print("KB6 colors after K1: " + colors())
print("KB6 DONE")