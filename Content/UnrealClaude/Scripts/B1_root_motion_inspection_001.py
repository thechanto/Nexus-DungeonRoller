import unreal

L = []
def p(s):
    L.append(str(s))
    unreal.log("RMINSPECT| " + str(s))

# ---- 1. The source AnimSequence -------------------------------------------
seq = unreal.EditorAssetLibrary.load_asset("/Game/RetargetedAbilities/Warrior/Ability_Ultimate")
p("SEQ=%s" % seq)
for prop in ["enable_root_motion", "root_motion_root_lock", "force_root_lock",
             "use_normalized_root_motion_scale"]:
    try:
        p("  seq.%s = %s" % (prop, seq.get_editor_property(prop)))
    except Exception as e:
        p("  seq.%s -> ERR %s" % (prop, e))

try:
    p("  seq.sequence_length = %s" % seq.get_editor_property("sequence_length"))
except Exception as e:
    p("  seq len ERR %s" % e)

# ---- 2. Root bone track: does the root actually translate? -----------------
# find the root bone name from the skeleton
skel = seq.get_editor_property("skeleton")
p("SKELETON=%s" % skel.get_path_name())
root_name = "root"
try:
    names = unreal.AnimationLibrary.get_animation_track_names(seq)
    p("  track_names(first 6)=%s  total=%d" % ([str(n) for n in names[:6]], len(names)))
    if len(names):
        root_name = str(names[0])
except Exception as e:
    p("  get_animation_track_names ERR %s" % e)

p("ROOT_BONE=%s" % root_name)
dur = 4.366667
p("ROOT POSE SAMPLES (bone=%s, extract_root_motion=False):" % root_name)
for i in range(0, 23):
    t = dur * i / 22.0
    try:
        xf = unreal.AnimationLibrary.get_bone_pose_for_time(seq, root_name, t, False)
        loc = xf.translation
        p("  t=%5.3f  X=%9.2f Y=%9.2f Z=%9.2f" % (t, loc.x, loc.y, loc.z))
    except Exception as e:
        p("  t=%5.3f  ERR %s" % (t, e))
        break

# ---- 3. The montage --------------------------------------------------------
mont = unreal.EditorAssetLibrary.load_asset("/Game/RetargetedAbilities/Warrior/AM_SkyCrusher")
p("MONTAGE=%s" % mont)
for prop in ["enable_root_motion_translation", "enable_root_motion_rotation",
             "enable_auto_blend_out", "blend_mode_in", "blend_mode_out",
             "root_motion_root_lock", "has_root_motion"]:
    try:
        p("  mont.%s = %s" % (prop, mont.get_editor_property(prop)))
    except Exception as e:
        p("  mont.%s -> ERR %s" % (prop, e))
try:
    p("  mont.HasRootMotion() = %s" % mont.has_root_motion())
except Exception as e:
    p("  mont.has_root_motion() ERR %s" % e)

# ---- 4. BP_NexusPlayer: mesh -> AnimBP -> RootMotionMode -------------------
bpc = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/BP_NexusPlayer")
if bpc is None:
    for cand in ["/Game/Characters/BP_NexusPlayer", "/Game/BP_NexusPlayer",
                 "/Game/Player/BP_NexusPlayer"]:
        bpc = unreal.EditorAssetLibrary.load_blueprint_class(cand)
        if bpc:
            p("PLAYER_BP found at %s" % cand)
            break
else:
    p("PLAYER_BP found at /Game/Blueprints/BP_NexusPlayer")

if bpc:
    cdo = unreal.get_default_object(bpc)
    p("PLAYER_CDO=%s" % cdo)
    mesh = cdo.get_editor_property("mesh")
    p("  mesh=%s" % mesh)
    anim_class = mesh.get_editor_property("anim_class")
    p("  mesh.anim_class=%s" % anim_class)
    if anim_class:
        acdo = unreal.get_default_object(anim_class)
        p("  ABP_CDO=%s" % acdo)
        try:
            rmm = acdo.get_editor_property("root_motion_mode")
            p("  *** ABP.root_motion_mode = %s ***" % rmm)
        except Exception as e:
            p("  ABP.root_motion_mode ERR %s" % e)
    cmc = cdo.get_editor_property("character_movement")
    p("  CMC=%s" % cmc)
    for prop in ["gravity_scale", "max_walk_speed", "jump_z_velocity"]:
        try:
            p("    cmc.%s = %s" % (prop, cmc.get_editor_property(prop)))
        except Exception as e:
            p("    cmc.%s ERR %s" % (prop, e))
else:
    p("PLAYER_BP NOT FOUND -- searching")
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    for a in ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/Engine", "Blueprint")):
        n = str(a.asset_name)
        if "NexusPlayer" in n or "Player" in n:
            p("   candidate: %s" % a.package_name)

with open("C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/b1_rootmotion_out.txt", "w") as f:
    f.write("\n".join(L))
p("WROTE b1_rootmotion_out.txt")
