import unreal
c = unreal.load_class(None, "/Script/Nexus.NexusLockOnComponent")
unreal.log("LOCKON02: class=" + str(c))
if c:
    cdo = unreal.get_default_object(c)
    unreal.log("LOCKON02: range=" + str(cdo.get_editor_property("lock_on_range")) + " socket=" + str(cdo.get_editor_property("target_socket_name")))
