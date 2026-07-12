import unreal

# 1) Save the duplicated widget (the one asset this script touches)
ok = unreal.EditorAssetLibrary.save_asset("/Game/Widgets/W_LockOnIndicator", only_if_is_dirty=False)
unreal.log("LOCKON04: widget_saved=" + str(ok))

# 2) Read-only: probe live-coded class reflection via FProperty helper (bypasses python wrapper)
lib = unreal.get_default_object(unreal.load_class(None, "/Script/Nexus.NexusAbilityUILibrary"))
comp_cls = unreal.load_class(None, "/Script/Nexus.NexusLockOnComponent")
unreal.log("LOCKON04: comp_cls=" + str(comp_cls))
for prop in ("LockOnRange", "TargetSocketName", "LockOnAction", "IndicatorWidgetClass"):
    try:
        txt = lib.call_method("GetClassDefaultPropertyText", (comp_cls, prop))
        unreal.log("LOCKON04: " + prop + "=" + str(txt))
    except Exception as e:
        unreal.log("LOCKON04: " + prop + "_FAILED " + str(e))

# 3) Read-only: capability probes for upcoming steps
bel = unreal.BlueprintEditorLibrary
unreal.log("LOCKON04: bel_fns=" + ",".join([n for n in dir(bel) if "interface" in n.lower() or "implement" in n.lower()]))
unreal.log("LOCKON04: DONE")
