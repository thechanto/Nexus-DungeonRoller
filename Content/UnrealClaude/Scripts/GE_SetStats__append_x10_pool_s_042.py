import unreal, re
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)
FX = "/Game/GameplayAbilitySystem/Effects/"
ge = FX + "GE_SetStats"
cls = unreal.load_object(None, ge + ".GE_SetStats_C")
txt = str(lib.call_method("GetClassDefaultPropertyText", (cls, "Modifiers")))
if re.search(r"ModifierOp=(Multiply\w*|Multiplicitive)", txt):
    raise RuntimeError("GE_SetStats already patched - aborting to avoid duplicates")

def split_mods(array_text):
    inner = array_text.strip()[1:-1]
    mods, depth, start = [], 0, 0
    for i, c in enumerate(inner):
        if c == "(": depth += 1
        elif c == ")": depth -= 1
        elif c == "," and depth == 0:
            mods.append(inner[start:i]); start = i + 1
    mods.append(inner[start:])
    return [m for m in mods if m.strip()]

# template: GE_UpgradeStat_Vitality's MaxHealth AddBase ScalableFloat modifier
tcls = unreal.load_object(None, FX + "GE_UpgradeStat_Vitality.GE_UpgradeStat_Vitality_C")
ttxt = str(lib.call_method("GetClassDefaultPropertyText", (tcls, "Modifiers")))
tpl = next(m for m in split_mods(ttxt) if 'AttributeName="MaxHealth"' in m)
m_add = re.search(r"ModifierOp=(\w+)", tpl)
add_op = m_add.group(1) if m_add else None
if add_op is None:
    tpl = tpl.replace("ModifierMagnitude=", "ModifierOp=AddBase,ModifierMagnitude=", 1)
    add_op = "AddBase"
mul_py = [n for n in dir(unreal.GameplayModOp) if "MULTIPLY" in n or "MULTIPLIC" in n]
mul_py = sorted(mul_py)  # MULTIPLY_ADDITIVE before MULTIPLY_COMPOUND
mul_op = "".join(w.capitalize() for w in mul_py[0].split("_"))
print(f"[STEP] template op={add_op}, multiply candidates={mul_py} -> using {mul_op}")

def make(attr, op, val):
    m = tpl.replace("MaxHealth", attr)
    m = re.sub(r"ModifierOp=\w+", "ModifierOp=" + op, m, count=1)
    m = re.sub(r"(ScalableFloatMagnitude=\(Value=)[-0-9.]+", r"\g<1>" + val, m, count=1)
    return m

appended = [
    make("MaxHealth", mul_op, "10.000000"),
    make("MaxHealth", add_op, "100.000000"),
    make("MaxStamina", mul_op, "10.000000"),
    make("MaxMana", mul_op, "10.000000"),
]
before_mods = split_mods(txt)
new_txt = "(" + ",".join(before_mods + appended) + ")"
if not lib.call_method("SetClassDefaultPropertyText", (cls, "Modifiers", new_txt)):
    raise RuntimeError("ImportText failed")

cdo = unreal.get_default_object(cls)
mods = cdo.get_editor_property("modifiers")
if len(mods) != len(before_mods) + 4:
    raise RuntimeError(f"mod count {len(mods)} != {len(before_mods) + 4}")
tail = []
for m in list(mods)[-4:]:
    a = m.get_editor_property("attribute").export_text()
    an = re.search(r'AttributeName="(\w+)"', a).group(1)
    op = str(m.get_editor_property("modifier_op"))
    v = m.get_editor_property("modifier_magnitude").get_editor_property("scalable_float_magnitude").get_editor_property("value")
    tail.append(f"{an}:{op.split('.')[-1].rstrip('>: 0123456789')}:{v}")
ok = ("MaxHealth" in tail[0] and ":10.0" in tail[0] and "MaxHealth" in tail[1] and "100.0" in tail[1]
      and "MaxStamina" in tail[2] and ":10.0" in tail[2] and "MaxMana" in tail[3] and ":10.0" in tail[3]
      and "MULTIPL" in tail[0] and "MULTIPL" in tail[2] and "MULTIPL" in tail[3] and "MULTIPL" not in tail[1])
if not ok:
    raise RuntimeError(f"verify failed: {tail}")
saved = unreal.EditorAssetLibrary.save_asset(ge, only_if_is_dirty=False)
print(f"[STEP-OK] GE_SetStats: appended {tail} saved={saved}")
