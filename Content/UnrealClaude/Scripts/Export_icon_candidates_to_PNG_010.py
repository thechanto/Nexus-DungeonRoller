import unreal, os

OUT = r"C:\Users\chant\AppData\Local\Temp\nexus_items\icons"
os.makedirs(OUT, exist_ok=True)

CANDIDATES = [
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_1_1024x1024", "potion_1"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_2_1024x1024", "potion_2"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_10_1024x1024", "potion_10"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_13_1024x1024", "potion_13"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_16_1024x1024", "potion_16"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_19_1024x1024", "potion_19"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_22_1024x1024", "potion_22"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_25_1024x1024", "potion_25"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_28_1024x1024", "potion_28"),
    ("/Game/100+Dark_Fantasy_Potion_Icons/1024x1024/T_Dark_Fantasy_Potion_100_1024x1024", "potion_100"),
    ("/Game/Widgets/Images/TalentTree/battle-axe", "axe"),
    ("/Game/Widgets/Images/TalentTree/crescent-staff", "staff"),
    ("/Game/StarterContent/Textures/T_Metal_Gold_D", "gold"),
]

opts = unreal.ImageWriteOptions()
opts.set_editor_property("format", unreal.DesiredImageFormat.PNG)
opts.set_editor_property("overwrite_file", True)
opts.set_editor_property("async_", False)

ok, fail = 0, []
for path, name in CANDIDATES:
    tex = unreal.EditorAssetLibrary.load_asset(path)
    if not tex:
        fail.append(name + ":load")
        continue
    try:
        unreal.ImageWriteBlueprintLibrary.export_to_disk(tex, os.path.join(OUT, name + ".png"), opts)
        ok += 1
    except Exception as e:
        fail.append(name + ":" + str(e)[:60])

unreal.log("ICON_EXPORT ok=%d fail=%s" % (ok, fail))
