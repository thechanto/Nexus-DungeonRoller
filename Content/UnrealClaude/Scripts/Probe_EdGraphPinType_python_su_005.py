import unreal

MARK = "LOOT04"
pt = unreal.EdGraphPinType()
unreal.log(MARK + ": dir = " + ", ".join([n for n in dir(pt) if not n.startswith("_")]))
try:
    unreal.log(MARK + ": export = " + pt.export_text())
except Exception as e:
    unreal.log(MARK + ": export failed " + str(e))
unreal.log(MARK + ": doc = " + str(unreal.EdGraphPinType.__doc__)[:800])
try:
    unreal.log(MARK + ": init doc = " + str(unreal.EdGraphPinType.__init__.__doc__)[:600])
except Exception as e:
    pass
