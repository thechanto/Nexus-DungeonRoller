import unreal
w = unreal.EditorLevelLibrary.get_editor_world()
print("CURRENT LEVEL: %s" % w.get_path_name())
