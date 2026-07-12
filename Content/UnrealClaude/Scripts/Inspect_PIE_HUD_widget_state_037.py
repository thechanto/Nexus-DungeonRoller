
import unreal
out = []
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
out.append('WORLD: %s' % (world.get_name() if world else 'NONE'))
if world:
    pc = unreal.GameplayStatics.get_player_controller(world, 0)
    out.append('PC: %s' % pc.get_name())
    comp_cls = unreal.load_class(None, '/Script/Nexus.NexusInventoryUIComponent')
    comp = pc.get_component_by_class(comp_cls) if comp_cls else None
    out.append('InventoryUI comp: %s' % (comp.get_name() if comp else 'MISSING'))
    if comp:
        fns = [f for f in dir(comp) if any(k in f.lower() for k in ('invent','menu','toggle','open','close','hud'))]
        out.append('comp fns: %s' % fns)
    wrap_cls = unreal.load_class(None, '/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD_C')
    hud_cls = unreal.load_class(None, '/Game/Widgets/W_PlayerHUD.W_PlayerHUD_C')
    wraps = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, wrap_cls, False)
    huds = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, hud_cls, False)
    out.append('W_NexusNarrativeHUD instances: %s' % [w.get_name() for w in wraps])
    out.append('W_PlayerHUD instances: %s' % [(w.get_name(), str(w.get_visibility()), w.is_in_viewport()) for w in huds])
print('NEXUS_VERIFY_A|' + '||'.join(out))
