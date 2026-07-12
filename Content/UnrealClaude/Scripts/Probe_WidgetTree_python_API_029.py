import unreal
tree = unreal.load_object(None, "/Game/Widgets/W_NexusNarrativeHUD.W_NexusNarrativeHUD:WidgetTree")
print("[PROBE] dir:", [a for a in dir(tree) if not a.startswith("_")])
print("[PROBE] doc:", unreal.WidgetTree.__doc__)
