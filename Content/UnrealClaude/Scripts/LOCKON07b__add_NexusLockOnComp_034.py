import unreal

BP = "/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer"
bp = unreal.load_asset(BP)
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
BFL = unreal.SubobjectDataBlueprintFunctionLibrary

def get_data(h):
    for holder in (sds, BFL):
        for name in ("k2_find_subobject_data_from_handle", "find_subobject_data_from_handle", "get_data"):
            if hasattr(holder, name):
                return getattr(holder, name)(h)
    raise Exception("no data finder; sds=[%s] BFL=[%s]" % (
        ",".join(n for n in dir(sds) if "find" in n or "data" in n),
        ",".join(n for n in dir(BFL) if not n.startswith("_"))))

handles = sds.k2_gather_subobject_data_for_blueprint(bp)
unreal.log("LOCKON07: gathered %d handles" % len(handles))
comp_cls = unreal.load_class(None, "/Script/Nexus.NexusLockOnComponent")

existing = None
for h in handles:
    obj = BFL.get_object(get_data(h))
    if obj and obj.get_class().get_name() == "NexusLockOnComponent":
        existing = h
        unreal.log("LOCKON07: component already present, skipping add")
        break

if existing is None:
    params = unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=comp_cls, blueprint_context=bp)
    res = sds.add_new_subobject(params)
    new_handle = res[0] if isinstance(res, tuple) else res
    fail = res[1] if isinstance(res, tuple) and len(res) > 1 else ""
    unreal.log("LOCKON07: add_new_subobject fail_reason='%s'" % str(fail))
    try:
        sds.rename_subobject(new_handle, unreal.Text("LockOn"))
        unreal.log("LOCKON07: renamed to LockOn")
    except Exception as e:
        unreal.log("LOCKON07: rename failed (cosmetic): %s" % str(e))
    existing = new_handle

tmpl = BFL.get_object(get_data(existing))
unreal.log("LOCKON07: template=" + str(tmpl))
ia = unreal.load_asset("/Game/Input/Actions/IA_LockOn")
wcls = unreal.load_class(None, "/Game/Widgets/W_LockOnIndicator.W_LockOnIndicator_C")
for prop, val in (("lock_on_action", ia), ("LockOnAction", ia)):
    try:
        tmpl.set_editor_property(prop, val)
        unreal.log("LOCKON07: set %s OK" % prop)
        break
    except Exception as e:
        unreal.log("LOCKON07: set %s failed: %s" % (prop, str(e)))
for prop, val in (("indicator_widget_class", wcls), ("IndicatorWidgetClass", wcls)):
    try:
        tmpl.set_editor_property(prop, val)
        unreal.log("LOCKON07: set %s OK" % prop)
        break
    except Exception as e:
        unreal.log("LOCKON07: set %s failed: %s" % (prop, str(e)))

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
ok = unreal.EditorAssetLibrary.save_asset(BP, only_if_is_dirty=False)
unreal.log("LOCKON07: compiled, saved=" + str(ok))
