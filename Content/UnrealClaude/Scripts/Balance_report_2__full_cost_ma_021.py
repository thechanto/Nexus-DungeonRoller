import unreal

def full_mag(bp_path):
    name = bp_path.split("/")[-1]
    cls = unreal.load_object(None, bp_path + "." + name + "_C")
    cdo = unreal.get_default_object(cls)
    mods = cdo.get_editor_property("modifiers")
    for i, m in enumerate(mods):
        attr = m.get_editor_property("attribute").get_editor_property("attribute_name")
        mag = m.get_editor_property("modifier_magnitude")
        txt = mag.export_text()
        # print in chunks to dodge any log truncation
        for j in range(0, len(txt), 400):
            print("MAG|%s|mod%d|%s|part%d|%s" % (name, i, attr, j // 400, txt[j:j+400]))

print("=== FULL MAGNITUDES ===")
for p in ["/Game/GameplayAbilitySystem/Effects/GE_Cost_Stamina",
          "/Game/GameplayAbilitySystem/Effects/GE_Cost_Mana",
          "/Game/GameplayAbilitySystem/Effects/GE_Dash_Cost",
          "/Game/GameplayAbilitySystem/Effects/GE_Damage_Instant"]:
    full_mag(p)

print("=== SAVE GAME DUMP ===")
save = unreal.GameplayStatics.load_game_from_slot("BP_ThirdPersonPlayerController_C_0", 0)
if not save:
    print("SAVE: not found")
else:
    print("SAVE class: %s" % save.get_class().get_name())
    for prop in ["PlayerTalentData", "AssignedAbilities", "UnlockedAbilities"]:
        try:
            v = save.get_editor_property(prop)
            try:
                txt = v.export_text()
            except Exception:
                txt = str(v)
            for j in range(0, len(txt), 400):
                print("SAVE|%s|part%d|%s" % (prop, j // 400, txt[j:j+400]))
        except Exception as e:
            print("SAVE|%s|ERR|%s" % (prop, e))
print("=== DONE dump2 ===")
