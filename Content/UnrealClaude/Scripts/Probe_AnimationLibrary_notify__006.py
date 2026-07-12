import unreal
cands=[n for n in dir(unreal.AnimationLibrary) if "notify" in n.lower() or "track" in n.lower()]
print("PROBE_BEGIN")
for c in cands: print("AL: "+c)
try:
    import inspect
    print("SIG add:", unreal.AnimationLibrary.add_anim_notify_event.__doc__)
except Exception as e: print("SIGERR", e)
print("PROBE_END")