import unreal
path = '/Game/GameplayAbilitySystem/Effects/GE_Cost_Mana_Ability'
cls = unreal.EditorAssetLibrary.load_blueprint_class(path)
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)
txt = str(lib.call_method('GetClassDefaultPropertyText', (cls, 'Modifiers')))
print('BEFORE=' + txt)
new = txt.replace('-33.000000', '-10.000000')
if new == txt:
    raise RuntimeError('no -33 value found; aborting without changes')
ok = lib.call_method('SetClassDefaultPropertyText', (cls, 'Modifiers', new))
print('SET_OK=' + str(ok))
after = str(lib.call_method('GetClassDefaultPropertyText', (cls, 'Modifiers')))
print('AFTER=' + after)
if '-10.000000' not in after:
    raise RuntimeError('verify failed: -10 not present after import')
saved = unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
print('SAVED=' + str(saved))
