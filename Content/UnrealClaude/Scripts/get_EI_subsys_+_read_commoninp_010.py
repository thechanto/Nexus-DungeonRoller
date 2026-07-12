import unreal
def mark(k,v): unreal.log("S10|%s|%s"%(k,v))
gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
gi=unreal.GameplayStatics.get_game_instance(gw)
mark("GI", gi.get_name() if gi else "None")
lp=None
try:
    lps=gi.get_editor_property("local_players")
    lp=lps[0] if lps else None
except Exception as e:
    mark("LP_ERR", e)
mark("LP", lp.get_name() if lp else "None")
sub=None
if lp:
    for meth in ["get_subsystem"]:
        try:
            sub=lp.get_subsystem(unreal.EnhancedInputLocalPlayerSubsystem); break
        except Exception as e:
            mark("GETSUB_ERR", e)
mark("EI_SUBSYS", sub.get_name() if sub else "None")

# CommonInputSettings flag - dump reflected props containing 'nhanced'
try:
    cis=unreal.get_default_object(unreal.load_class(None,"/Script/CommonInput.CommonInputSettings"))
    txt=cis.export_text()
    import re
    for line in txt.splitlines():
        if 'nhanced' in line or 'EnableEnhanced' in line:
            mark("FLAG_LINE", line.strip())
except Exception as e:
    mark("FLAG_ERR", e)

mark("DONE","ok")
