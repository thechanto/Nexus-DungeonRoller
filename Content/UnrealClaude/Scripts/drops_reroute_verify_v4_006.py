import unreal
def mark(k,v): unreal.log("DROPVERIFY|%s|%s"%(k,v))

les=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.is_in_play_in_editor():
    mark("FATAL","not in PIE"); raise SystemExit

gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pawn=unreal.GameplayStatics.get_player_pawn(gw,0)
pc=unreal.GameplayStatics.get_player_controller(gw,0)
ps=pc.get_editor_property("player_state") if pc else None

run_inv=None
for holder in [ps,pawn,pc]:
    if not holder: continue
    for c in holder.get_components_by_class(unreal.NarrativeInventoryComponent):
        if 'RunInventory' in c.get_name(): run_inv=c
    if run_inv: break
mark("RUN_INV", run_inv.get_name() if run_inv else "None")

pot_cls=unreal.load_class(None,"/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")

def gold(): return unreal.NexusAbilityUILibrary.get_run_inventory_gold(gw)
def potion():
    t=0
    for it in run_inv.find_items_of_class(pot_cls):
        if it: t+=it.get_quantity()
    return t
def flask():
    try: return pawn.get_editor_property("HealthPotionCount ")
    except Exception:
        try: return pawn.get_editor_property("HealthPotionCount")
        except Exception: return "ERR"

def drop_and_collect(gold_share):
    # SpawnLootDrop spawns BP_LootPickup at pawn loc; overlap may auto-collect.
    p=unreal.NexusAbilityUILibrary.spawn_loot_drop(pawn, 1.0, gold_share)
    if p and unreal.SystemLibrary.is_valid(p):
        # not auto-collected -> run the real reroute manually
        unreal.NexusAbilityUILibrary.handle_loot_pickup(p, pawn)
        return "manual", (not unreal.SystemLibrary.is_valid(p))
    return "auto_overlap", True

g0,p0,f0=gold(),potion(),flask()
mark("START","gold=%s potion=%s flask=%s"%(g0,p0,f0))

path,destroyed=drop_and_collect(1.0)
g1=gold()
mark("GOLD_DROP","path=%s gold=%s delta=%s destroyed=%s"%(path,g1,g1-g0,destroyed))

path2,destroyed2=drop_and_collect(0.0)
p1=potion()
mark("POTION_DROP","path=%s potion=%s delta=%s destroyed=%s"%(path2,p1,p1-p0,destroyed2))

# HUD gold text
gt=None
try: gt=pc.get_editor_property("HudRef").get_editor_property("GoldText")
except Exception: gt=None
if not gt:
    try:
        for w in unreal.WidgetLibrary.get_all_widgets_of_class(gw, unreal.TextBlock, False):
            if 'Gold' in w.get_name(): gt=w; break
    except Exception as e: mark("HUD_FIND_ERR",e)
if gt:
    try: mark("HUD_GOLDTEXT","%s (name=%s)"%(gt.get_text(), gt.get_name()))
    except Exception as e: mark("HUD_GOLDTEXT_ERR",e)
else:
    mark("HUD_GOLDTEXT","not found")

# drink potion -> flask +1, stack -1
items=run_inv.find_items_of_class(pot_cls)
if items:
    run_inv.use_item(items[0])
    mark("DRINK","flask %s->%s  potion %s->%s"%(f0,flask(),p1,potion()))
else:
    mark("DRINK","no potion")

try:
    ci=unreal.get_default_object(unreal.load_class(None,"/Script/CommonInput.CommonInputSettings"))
    mark("ENH_INPUT", ci.get_editor_property("enable_enhanced_input_support"))
except Exception as e: mark("ENH_INPUT_ERR",e)

mark("DONE","ok")
