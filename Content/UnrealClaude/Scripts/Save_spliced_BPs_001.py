import unreal
paths = [
    "/Game/Test/BP_InteractionTest",
    "/Game/GameplayAbilitySystem/Abilities/GA_Death",
    "/Game/ThirdPerson/Blueprints/BP_NexusPlayerState",
]
for p in paths:
    ok = unreal.EditorAssetLibrary.save_asset(p, only_if_is_dirty=False)
    unreal.log_warning("NEXUS_SAVE %s -> %s" % (p, ok))
