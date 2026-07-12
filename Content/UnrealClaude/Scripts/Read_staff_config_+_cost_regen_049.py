import unreal

lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)

def prop_text(cls, name):
    try:
        return str(lib.call_method('GetClassDefaultPropertyText', (cls, name)))
    except Exception as e:
        return 'ERR:' + str(e)

print('=== STAFF WEAPON CONFIG ===')
try:
    staff_cls = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Weapons/BP_Weapon_Staff')
    staff_cdo = unreal.get_default_object(staff_cls)
    cfg = staff_cdo.get_editor_property('WeaponConfig')
    print('STAFF_CONFIG=' + cfg.export_text())
except Exception as e:
    print('STAFF ERR: ' + str(e))

print('=== FIRE/AIM GA COST CLASSES ===')
for ga in ['GA_ShootProjectile_Ice', 'GA_ShootProjectile_GreenFire', 'GA_AOEAttack', 'GA_Shield']:
    try:
        cls = unreal.EditorAssetLibrary.load_blueprint_class('/Game/GameplayAbilitySystem/Abilities/' + ga)
        cdo = unreal.get_default_object(cls)
        print(ga + ' COST=' + str(cdo.get_editor_property('cost_gameplay_effect_class')))
    except Exception as e:
        print(ga + ' ERR: ' + str(e))

print('=== COST + REGEN GE DETAILS ===')
for ge in ['GE_Cost_Mana', 'GE_Cost_Mana_Heavy', 'GE_Cost_Mana_Ability',
           'GE_Cost_Stamina', 'GE_Cost_Stamina_Heavy', 'GE_Cost_Stamina_Ability',
           'GE_ManaRegen', 'GE_Status_StaminaRegen', 'GE_Block_StaminaRegen']:
    path = '/Game/GameplayAbilitySystem/Effects/' + ge
    try:
        cls = unreal.EditorAssetLibrary.load_blueprint_class(path)
        print(ge + ' MODS=' + prop_text(cls, 'Modifiers'))
        print(ge + ' DURPOLICY=' + prop_text(cls, 'DurationPolicy'))
        print(ge + ' PERIOD=' + prop_text(cls, 'Period'))
        print(ge + ' DURMAG=' + prop_text(cls, 'DurationMagnitude'))
    except Exception as e:
        print(ge + ' ERR: ' + str(e))
print('=== DONE READS ===')
