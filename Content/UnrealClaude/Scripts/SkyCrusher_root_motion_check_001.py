import unreal

# Find AM_SkyCrusher wherever it lives
ar = unreal.AssetRegistryHelpers.get_asset_registry()
found = []
for a in ar.get_assets_by_class(unreal.TopLevelAssetPath('/Script/Engine', 'AnimMontage')):
    n = str(a.asset_name)
    if 'SkyCrusher' in n:
        found.append(str(a.package_name))
print("MONTAGES:", found)

for path in found:
    m = unreal.load_asset(path)
    print("=== %s ===" % path)
    for prop in ['enable_root_motion', 'enable_root_motion_translation',
                 'enable_root_motion_rotation', 'root_motion_root_lock',
                 'force_root_lock', 'blend_mode', 'sequence_length', 'rate_scale']:
        try:
            print("   %-32s %s" % (prop, m.get_editor_property(prop)))
        except Exception:
            pass
    # Source sequences inside the slot tracks
    try:
        for st in m.get_editor_property('slot_anim_tracks'):
            slot = st.get_editor_property('slot_name')
            seg = st.get_editor_property('anim_track').get_editor_property('anim_segments')
            for s in seg:
                ref = s.get_editor_property('anim_reference')
                print("   SLOT %s -> %s" % (slot, ref.get_path_name() if ref else None))
                if ref:
                    for prop in ['enable_root_motion', 'root_motion_root_lock', 'force_root_lock']:
                        try:
                            print("      seq.%-28s %s" % (prop, ref.get_editor_property(prop)))
                        except Exception:
                            pass
    except Exception as e:
        print("   slot err", e)

# Anim BP root motion mode
for a in ar.get_assets_by_class(unreal.TopLevelAssetPath('/Script/Engine', 'AnimBlueprint')):
    n = str(a.asset_name)
    if 'Player' in n or 'Nexus' in n or 'Manny' in n or 'Quinn' in n:
        bp = unreal.load_asset(str(a.package_name))
        try:
            cdo = unreal.get_default_object(bp.generated_class())
            print("ABP %s root_motion_mode = %s" % (n, cdo.get_editor_property('root_motion_mode')))
        except Exception as e:
            print("ABP %s err %s" % (n, e))
