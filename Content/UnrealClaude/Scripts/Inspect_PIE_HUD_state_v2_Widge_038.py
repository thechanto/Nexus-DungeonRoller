
import unreal
out = []
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
out.append('WORLD:%s' % (world.get_name() if world else 'NONE'))
if world:
    pc = unreal.GameplayStatics.get_player_controller(world, 0)
    comp_cls = unreal.load_class(None, '/Script/Nexus.NexusInventoryUIComponent')
    comp = pc.get_component_by_class(comp_cls)
    out.append('COMP:%s' % (comp.get_name() if comp else 'MISSING'))
    wrap_cls = unreal.load_class(None, '/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD_C')
    hud_cls = unreal.load_class(None, '/Game/Widgets/W_PlayerHUD.W_PlayerHUD_C')
    menu_cls = unreal.load_class(None, '/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C')
    wl = unreal.WidgetLibrary
    wraps = wl.get_all_widgets_of_class(world, wrap_cls, False)
    huds = wl.get_all_widgets_of_class(world, hud_cls, False)
    menus = wl.get_all_widgets_of_class(world, menu_cls, False)
    out.append('WRAP:%s' % [(w.get_name(), w.is_in_viewport()) for w in wraps])
    out.append('PLAYERHUD:%s' % [(w.get_name(), str(w.get_visibility()), w.is_in_viewport()) for w in huds])
    out.append('MENU:%s' % [(w.get_name(), str(w.get_visibility())) for w in menus])
    if comp:
        fns = [f for f in dir(comp) if any(k in f.lower() for k in ('invent','menu','toggle','close','open'))]
        out.append('FNS:%s' % fns)
print('NEXUS_VA|' + ' || '.join(out))
