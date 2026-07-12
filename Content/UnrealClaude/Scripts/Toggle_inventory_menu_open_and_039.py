
import unreal
out = []
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc = unreal.GameplayStatics.get_player_controller(world, 0)
comp = pc.get_component_by_class(unreal.load_class(None, '/Script/Nexus.NexusInventoryUIComponent'))
comp.call_method('ToggleInventoryMenu')
menu_cls = unreal.load_class(None, '/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C')
menus = unreal.WidgetLibrary.get_all_widgets_of_class(world, menu_cls, False)
out.append('MENUS:%s' % [(m.get_name(), str(m.get_visibility())) for m in menus])
if menus:
    m = menus[0]
    props = [p for p in dir(m) if 'invent' in p.lower()]
    out.append('MENUPROPS:%s' % props)
    for p in props:
        try:
            out.append('%s=%s' % (p, m.get_editor_property(p)))
        except Exception as e:
            out.append('%s=ERR' % p)
out.append('SHOWCURSOR:%s' % pc.get_editor_property('show_mouse_cursor'))
hud = unreal.WidgetLibrary.get_all_widgets_of_class(world, unreal.load_class(None, '/Game/Widgets/W_PlayerHUD.W_PlayerHUD_C'), False)
out.append('PLAYERHUD:%s' % [(w.get_name(), str(w.get_visibility())) for w in hud])
ps = pc.player_state
run_inv = None
for c in ps.get_components_by_class(unreal.load_class(None, '/Script/NarrativeInventory.NarrativeInventoryComponent')):
    if c.get_name() == 'RunInventory':
        run_inv = c
items = run_inv.call_method('GetItems') if run_inv else []
out.append('RUNITEMS:%s' % [(i.get_class().get_name(), i.get_editor_property('quantity')) for i in items])
print('NEXUS_VB|' + ' || '.join(out))
