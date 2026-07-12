import unreal
print("PROBE2_BEGIN")
hits=[n for n in dir(unreal) if ("struct" in n.lower() and "editor" in n.lower()) or "UserDefinedStruct" in n]
for h in hits: print("U: "+h)
print("PROBE2_END")