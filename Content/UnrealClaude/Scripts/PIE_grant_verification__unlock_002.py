import unreal
lib = unreal.NexusAbilityUILibrary
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
player = unreal.GameplayStatics.get_player_character(world, 0)
unreal.log("GT world=%s player=%s" % (world.get_name(), player.get_name() if player else None))

base = "/Game/GameplayAbilitySystem/Abilities/DataAssets"
names = ["Warrior/DA_Ability_Warrior_ShieldSlam","Warrior/DA_Ability_Warrior_SwordSweep",
         "Warrior/DA_Ability_Warrior_LeapSlash","Warrior/DA_Ability_Warrior_SkyCrusher",
         "Mage/DA_Ability_Mage_ArcaneBolt","Mage/DA_Ability_Mage_CosmicRift",
         "Mage/DA_Ability_Mage_Meteor","Mage/DA_Ability_Mage_Burden"]
das = []
for n in names:
    short = n.split("/")[-1]
    c = unreal.load_class(None, "%s/%s.%s_C" % (base, n, short))
    das.append(c)
    unreal.log("GT load %s -> %s" % (short, c))

unreal.log("GT assigned_before=%s" % (list(lib.get_assigned_abilities(world)),))
for c in das:
    if c and not lib.is_ability_unlocked(world, c):
        lib.unlock_ability(world, c)

# round 1: slots 0-3 = ShieldSlam, SwordSweep, ArcaneBolt, CosmicRift
r1 = [das[0], das[1], das[4], das[5]]
for i, c in enumerate(r1):
    ok = lib.assign_ability_to_slot(world, c, i)
    unreal.log("GT assign r1 slot %d %s -> %s" % (i, c.get_name(), ok))
g1 = lib.grant_assigned_abilities(player)
unreal.log("GT GRANT_ROUND1=%d (expect 4)" % g1)

# round 2: swap all four slots = LeapSlash, SkyCrusher, Meteor, Burden
r2 = [das[2], das[3], das[6], das[7]]
for i, c in enumerate(r2):
    ok = lib.assign_ability_to_slot(world, c, i)
    unreal.log("GT assign r2 slot %d %s -> %s" % (i, c.get_name(), ok))
g2 = lib.grant_assigned_abilities(player)
unreal.log("GT GRANT_ROUND2=%d (expect 4)" % g2)

# try to inspect spec InputIDs
try:
    asc = player.get_component_by_class(unreal.AbilitySystemComponent)
    cont = asc.get_editor_property("activatable_abilities")
    items = cont.get_editor_property("items")
    for s in items:
        try:
            unreal.log("GT SPEC input_id=%s ability=%s" % (s.get_editor_property("input_id"), s.get_editor_property("ability")))
        except Exception as e:
            unreal.log("GT SPEC introspect fail: %s" % e)
except Exception as e:
    unreal.log("GT ASC introspect fail: %s" % e)
unreal.log("GT_DONE")
