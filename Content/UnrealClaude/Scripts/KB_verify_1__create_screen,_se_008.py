import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
print("KB1 world=" + str(world))
cls = unreal.load_object(None, "/Game/Widgets/W_AbilitiesScreen.W_AbilitiesScreen_C")
screen = unreal.WidgetBlueprintLibrary.create(world, cls, None)
screen.add_to_viewport(100)
print("KB1 screen added: " + screen.get_name())
lib = unreal.NexusAbilityUILibrary
# report initial keybind button state (should be disabled/grey: nothing selected)
for i in range(1, 5):
    btn = screen.get_widget_from_name("Keybind" + str(i))
    print("KB1 init Keybind%d enabled=%s color=%s" % (i, btn.get_is_enabled(), btn.background_color))
# find an ability slot whose AbilityData is unlocked and click it via its dispatcher
slot_cls = unreal.load_object(None, "/Game/Widgets/W_AbilitySlot.W_AbilitySlot_C")
picked = None
for name in ["WarriorSlot_LeapSlash", "WarriorSlot_SkyCrusher", "MageSlot_Meteor", "MageSlot_Burden", "WarriorSlot_ShieldSlam"]:
    sw = screen.get_widget_from_name(name)
    if not sw: continue
    data = sw.get_editor_property("AbilityData")
    unlocked = lib.is_ability_unlocked(world, data) if data else False
    print("KB1 slot %s data=%s unlocked=%s" % (name, data.get_name() if data else "None", unlocked))
    if unlocked and picked is None: picked = (sw, data)
if picked:
    sw, data = picked
    disp = sw.get_editor_property("OnAbilitySlotClicked")
    disp.broadcast(None)
    print("KB1 clicked slot for " + data.get_name())
    print("KB1 selected=" + str(lib.get_selected_ability(screen)))
    print("KB1 assigned_slot=" + str(lib.get_assigned_slot_for_ability(world, data)))
    for i in range(1, 5):
        btn = screen.get_widget_from_name("Keybind" + str(i))
        print("KB1 post-select Keybind%d enabled=%s color=%s" % (i, btn.get_is_enabled(), btn.background_color))
else:
    print("KB1 ERROR no unlocked slot found")
print("KB1 DONE")