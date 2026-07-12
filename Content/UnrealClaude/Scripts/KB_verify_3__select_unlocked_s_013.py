import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
screen = unreal.find_object(None, "/Engine/Transient.UnrealEdEngine_0:BP_MyGameInstance_C_1.W_AbilitiesScreen_C_0")
print("KB3 screen=" + str(screen))
lib = unreal.NexusAbilityUILibrary
for i in range(1, 5):
    btn = screen.get_widget_from_name("Keybind" + str(i))
    print("KB3 init Keybind%d enabled=%s" % (i, btn.get_is_enabled()))
picked = None
for name in ["WarriorSlot_LeapSlash", "WarriorSlot_SkyCrusher", "MageSlot_Meteor", "MageSlot_Burden", "WarriorSlot_ShieldSlam"]:
    sw = screen.get_widget_from_name(name)
    if not sw:
        print("KB3 slot %s NOT FOUND" % name)
        continue
    data = sw.get_editor_property("AbilityData")
    unlocked = lib.is_ability_unlocked(world, data) if data else False
    print("KB3 slot %s data=%s unlocked=%s" % (name, data.get_name() if data else "None", unlocked))
    if unlocked and picked is None: picked = (sw, data)
if picked:
    sw, data = picked
    sw.get_editor_property("OnAbilitySlotClicked").broadcast(None)
    print("KB3 clicked slot, selected=" + str(lib.get_selected_ability(screen)))
    print("KB3 assigned_slot_before=" + str(lib.get_assigned_slot_for_ability(world, data)))
    for i in range(1, 5):
        btn = screen.get_widget_from_name("Keybind" + str(i))
        c = btn.background_color
        print("KB3 post-select Keybind%d enabled=%s color=(%.3f,%.3f,%.3f)" % (i, btn.get_is_enabled(), c.r, c.g, c.b))
else:
    print("KB3 ERROR no unlocked slot")
print("KB3 DONE")