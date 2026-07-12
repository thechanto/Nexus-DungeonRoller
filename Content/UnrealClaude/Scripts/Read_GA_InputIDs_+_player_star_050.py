import unreal

print('=== GA INPUT IDS ===')
gas = ['GA_ShootProjectile_Ice', 'GA_ShootProjectile_GreenFire', 'GA_AOEAttack', 'GA_Shield',
       'GA_MeleeAttack_AxeCombo', 'GA_MeleeAttack_AxeSwing', 'GA_Dash', 'GA_FillShield']
for ga in gas:
    try:
        cls = unreal.EditorAssetLibrary.load_blueprint_class('/Game/GameplayAbilitySystem/Abilities/' + ga)
        cdo = unreal.get_default_object(cls)
        print(ga + ' INPUTID=' + str(cdo.get_editor_property('AbilityInputID')))
    except Exception as e:
        print(ga + ' ERR: ' + str(e))

print('=== PLAYER STARTING ABILITIES ===')
try:
    pcls = unreal.EditorAssetLibrary.load_blueprint_class('/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer')
    pcdo = unreal.get_default_object(pcls)
    for name in ['StartingAbilities', 'StartingEffects']:
        try:
            print(name + '=' + str(pcdo.get_editor_property(name)))
        except Exception as e:
            print(name + ' ERR: ' + str(e))
except Exception as e:
    print('PLAYER ERR: ' + str(e))
print('=== DONE INPUTIDS ===')
