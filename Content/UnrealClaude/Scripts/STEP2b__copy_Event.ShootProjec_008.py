import unreal

al = unreal.AnimationLibrary
eal = unreal.EditorAssetLibrary
res = []

def find_send_event_notify(montage):
    for ev in al.get_animation_notify_events(montage):
        try:
            n = ev.get_editor_property("notify")
        except Exception:
            n = None
        if n is not None and "SendGameplayEvent" in n.get_class().get_name():
            return n
    return None

ref = eal.load_asset("/Game/Animations/Staff/Montage_Staff_ShootProjectile")
ref_notify = find_send_event_notify(ref)
if ref_notify is None:
    raise Exception("Reference AN_SendGameplayEvent not found on staff montage")

tag = None
prop_used = None
for pname in ("Event Tag", "EventTag", "Event_Tag"):
    try:
        tag = ref_notify.get_editor_property(pname)
        prop_used = pname
        break
    except Exception:
        continue
if tag is None:
    raise Exception("Could not read Event Tag from reference notify")
res.append("REF tag=%s via prop '%s'" % (tag, prop_used))

for path in (
    "/Game/RetargetedAbilities/Mage/AM_ArcaneBolt",
    "/Game/RetargetedAbilities/Mage/AM_CosmicRift",
    "/Game/RetargetedAbilities/Mage/AM_Meteor",
    "/Game/RetargetedAbilities/Mage/AM_Burden",
):
    try:
        m = eal.load_asset(path)
        n = find_send_event_notify(m)
        if n is None:
            res.append("FAIL %s: no SendGameplayEvent notify found" % path)
            continue
        n.set_editor_property(prop_used, tag)
        readback = n.get_editor_property(prop_used)
        ok = eal.save_asset(path)
        res.append("SET %s tag=%s saved=%s" % (path, readback, ok))
    except Exception as e:
        res.append("ERROR %s: %s" % (path, str(e)[:200]))

print("STEP2B_BEGIN")
for r in res:
    print(r)
print("STEP2B_END")
