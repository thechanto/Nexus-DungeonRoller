import unreal

BP = "/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer"
bp = unreal.load_asset(BP)
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sds.k2_gather_subobject_data_for_blueprint(bp)
unreal.log("LOCKON07: gathered %d handles" % len(handles))

comp_cls = unreal.load_class(None, "/Script/Nexus.NexusLockOnComponent")

# guard: bail if a LockOn component already exists (rerun safety)
existing = None
for h in handles:
    d = unreal.SubobjectDataBlueprintFunctionLibrary.k2_find_subobject_data_from_handle(h)
    obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
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
    sds.rename_subobject(new_handle, unreal.Text("LockOn"))
    existing = new_handle

# best-effort template property fill (C++ BeginPlay fallback covers failure)
d = unreal.SubobjectDataBlueprintFunctionLibrary.k2_find_subobject_data_from_handle(existing)
tmpl = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
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
