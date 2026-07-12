import unreal
print("KBPROBE WidgetLibrary:" + str([n for n in dir(unreal.WidgetLibrary) if "crea" in n.lower()]))
print("KBPROBE modnames:" + str([n for n in dir(unreal) if "widget" in n.lower() and "lib" in n.lower()]))
import inspect
try:
    print("KBPROBE UserWidget create:" + str([n for n in dir(unreal.UserWidget) if "crea" in n.lower()]))
except Exception as e:
    print("KBPROBE err " + str(e))
print("KBPROBE DONE")