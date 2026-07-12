import unreal

al = unreal.AnimationLibrary
eal = unreal.EditorAssetLibrary

hit_start = eal.load_blueprint_class("/Game/AnimNotifies/AN_HitScanStart")
hit_end = eal.load_blueprint_class("/Game/AnimNotifies/AN_HitScanEnd")
send_event = eal.load_blueprint_class("/Game/AnimNotifies/AN_SendGameplayEvent")

res = []

def ensure_track(m):
    names = list(al.get_animation_notify_track_names(m))
    if len(names) == 0:
        al.add_animation_notify_track(m, "1", unreal.LinearColor(0.5, 0.5, 0.5, 1.0))
        names = ["1"]
    return str(names[0])

melee = [
    ("/Game/RetargetedAbilities/Warrior/AM_ShieldSlam", 0.30, 0.60),
    ("/Game/RetargetedAbilities/Warrior/AM_SwordSweep", 0.40, 0.85),
    ("/Game/RetargetedAbilities/Warrior/AM_LeapSlash", 0.90, 1.40),
    ("/Game/RetargetedAbilities/Warrior/AM_SkyCrusher", 2.60, 3.40),
]
for path, t0, t1 in melee:
    try:
        m = eal.load_asset(path)
        track = ensure_track(m)
        existing = list(al.get_animation_notify_events(m))
        if existing:
            res.append("SKIP %s already has %d notifies" % (path, len(existing)))
            continue
        n0 = al.add_animation_notify_event(m, track, float(t0), hit_start)
        n1 = al.add_animation_notify_event(m, track, float(t1), hit_end)
        ok = eal.save_asset(path)
        res.append("MELEE %s start@%.2f=%s end@%.2f=%s saved=%s" %
                   (path, t0, n0 is not None, t1, n1 is not None, ok))
    except Exception as e:
        res.append("ERROR %s: %s" % (path, str(e)[:200]))

def make_tag():
    try:
        t = unreal.GameplayTag()
        t.set_editor_property("tag_name", "Event.ShootProjectile")
        return t, "set_editor_property"
    except Exception:
        pass
    try:
        return unreal.GameplayTag(tag_name="Event.ShootProjectile"), "ctor"
    except Exception:
        return None, "FAILED"

ranged = [
    ("/Game/RetargetedAbilities/Mage/AM_ArcaneBolt", 0.65),
    ("/Game/RetargetedAbilities/Mage/AM_CosmicRift", 0.90),
    ("/Game/RetargetedAbilities/Mage/AM_Meteor", 0.85),
    ("/Game/RetargetedAbilities/Mage/AM_Burden", 0.60),
]
for path, t0 in ranged:
    try:
        m = eal.load_asset(path)
        track = ensure_track(m)
        existing = list(al.get_animation_notify_events(m))
        if existing:
            res.append("SKIP %s already has %d notifies" % (path, len(existing)))
            continue
        n = al.add_animation_notify_event(m, track, float(t0), send_event)
        tagset = "no_notify"
        if n is not None:
            tag, how = make_tag()
            if tag is not None:
                done = False
                last = ""
                for pname in ("Event Tag", "EventTag", "Event_Tag"):
                    try:
                        n.set_editor_property(pname, tag)
                        tagset = "%s via %s" % (pname, how)
                        done = True
                        break
                    except Exception as e:
                        last = str(e)[:80]
                if not done:
                    tagset = "TAGPROP_FAIL %s" % last
            else:
                tagset = "TAGMAKE_FAIL"
        ok = eal.save_asset(path)
        res.append("RANGED %s event@%.2f notify=%s tag=%s saved=%s" %
                   (path, t0, n is not None, tagset, ok))
    except Exception as e:
        res.append("ERROR %s: %s" % (path, str(e)[:200]))

unreal.AssetToolsHelpers.get_asset_tools().export_assets([
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Mage/AM_ArcaneBolt",
], "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/ClaudeScripts/export")

print("STEP2_BEGIN")
for r in res:
    print(r)
print("STEP2_END")
