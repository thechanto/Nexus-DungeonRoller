import unreal, json
OUT = r"C:\Users\chant\OneDrive\Documents\Unreal Projects\Nexus-part-final 5.7\Saved\nexus_probe_result.json"
res = {}
res["SubobjectDataBlueprintFunctionLibrary"] = [m for m in dir(unreal.SubobjectDataBlueprintFunctionLibrary) if not m.startswith("_")]
res["SubobjectDataSubsystem"] = [m for m in dir(unreal.SubobjectDataSubsystem) if not m.startswith("_")]
res["inv_props"] = [str(p) for p in dir(unreal.NarrativeInventoryComponent) if not p.startswith("_")]
with open(OUT, "w") as f:
    json.dump(res, f, indent=2)
print("WROTE")
