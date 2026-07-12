import unreal

MARK = "LOOT02"
names = [n for n in dir(unreal.BlueprintEditorLibrary) if "variable" in n.lower() or "member" in n.lower()]
unreal.log(MARK + ": BlueprintEditorLibrary variable fns: " + ", ".join(names))
try:
    import inspect
    for n in names:
        fn = getattr(unreal.BlueprintEditorLibrary, n)
        unreal.log(MARK + ": " + n + " doc: " + str(fn.__doc__)[:400])
except Exception as e:
    unreal.log(MARK + ": doc dump failed " + str(e))
