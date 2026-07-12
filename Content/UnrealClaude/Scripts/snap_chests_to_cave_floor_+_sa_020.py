import unreal

def mark(k, *v):
    unreal.log("CHESTMARK|%s|%s" % (k, " ".join(str(x) for x in v)))

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
all_actors = eas.get_all_level_actors()

chests = [a for a in all_actors if a.get_actor_label() and "TreasureChest_" in a.get_actor_label()]
mark("FOUND_CHESTS", len(chests), [c.get_actor_label() for c in chests])

for c in chests:
    loc = c.get_actor_location()
    start = unreal.Vector(loc.x, loc.y, -1850.0)
    end = unreal.Vector(loc.x, loc.y, -2400.0)
    res = unreal.SystemLibrary.line_trace_single(
        world, start, end,
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [c],
        unreal.DrawDebugTrace.NONE, True, unreal.LinearColor.RED, unreal.LinearColor.GREEN, 0.0)
    # res may be (bool, hit) or hit depending on binding
    hit = None
    if isinstance(res, tuple):
        ok, hit = res[0], res[1]
    else:
        hit = res
        ok = res is not None
    if hit is not None and ok:
        try:
            imp = hit.get_editor_property("location") if hasattr(hit, "get_editor_property") else hit.impact_point
        except Exception:
            imp = hit.impact_point
        newz = imp.z
        c.set_actor_location(unreal.Vector(loc.x, loc.y, newz), False, False)
        mark("SNAP", c.get_actor_label(), "floorZ=%.1f" % newz)
    else:
        mark("NOHIT", c.get_actor_label(), "kept z=%.1f" % loc.z)

saved = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
mark("LEVEL_SAVED", saved)
mark("DONE", "ok")
