
import unreal
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
allw = unreal.WidgetLibrary.get_all_widgets_of_class(world, unreal.UserWidget.static_class(), False)
print("TOTAL_USERWIDGETS:", len(allw))
menu=None; previews=[]; equipped=[]
for w in allw:
    cn = w.get_class().get_name()
    if cn.startswith("W_NarrativeMenu_Inventory"): menu=w
    if "PlayerPreview" in cn: previews.append((cn, w.is_visible()))
    if "EquippedItem" in cn:
        slot=None
        try: slot=str(w.get_editor_property("EquipmentSlot"))
        except Exception as e: slot="ERR:"+str(e)
        equipped.append((cn, slot, w.is_visible()))
print("MENU:", menu.get_name() if menu else None)
print("PLAYER_PREVIEWS:", previews)
print("EQUIPPED_ITEM_WIDGETS:", len(equipped))
for e in equipped: print("  SLOTW:", e)
if menu:
    for prop in ["EquipmentBox","PlayerPreview","WBP_PlayerPreview"]:
        try:
            v = menu.get_editor_property(prop)
            if v:
                par = v.get_parent()
                cc = v.get_children_count() if hasattr(v,"get_children_count") else "n/a"
                print("PROP", prop, "=>", v.get_name(), "parent:", (par.get_name() if par else None), "vis:", v.is_visible(), "children:", cc)
            else:
                print("PROP", prop, "=> None")
        except Exception as ex:
            print("PROP", prop, "ERR:", ex)
