import unreal
def mark(k,v): unreal.log("DROPVERIFY|%s|%s"%(k,v))

les=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.is_in_play_in_editor():
    mark("FATAL","not in PIE"); raise SystemExit

gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pawn=unreal.GameplayStatics.get_player_pawn(gw,0)
pc=unreal.GameplayStatics.get_player_controller(gw,0)
ps=pc.get_editor_property("player_state") if pc else None
mark("PAWN", pawn.get_name())

run_inv=None
for holder in [ps,pawn,pc]:
    if not holder: continue
    for c in holder.get_components_by_class(unreal.NarrativeInventoryComponent):
        if 'RunInventory' in c.get_name(): run_inv=c
    if run_inv: break
mark("RUN_INV", run_inv.get_name() if run_inv else "None")

pcls=unreal.load_class(None,"/Game/Loot/BP_LootPickup.BP_LootPickup_C")
gold_cls=unreal.load_class(None,"/Game/Inventory/Items/BP_Item_Gold.BP_Item_Gold_C")
pot_cls=unreal.load_class(None,"/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")

def qty(cls):
    t=0
    for it in run_inv.find_items_of_class(cls):
        if it: t+=it.get_quantity()
    return t
def flask():
    try: return pawn.get_editor_property("HealthPotionCount ")
    except Exception:
        try: return pawn.get_editor_property("HealthPotionCount")
        except Exception as e: return "ERR"

def spawn_pickup(loc,is_gold,gold_amt=25):
    xf=unreal.Transform(); xf.translation=loc
    a=None
    try:
        a=unreal.GameplayStatics.begin_deferred_actor_spawn_from_class(gw,pcls,xf,unreal.SpawnActorCollisionHandlingMethod.ALWAYS_SPAWN)
        try: a.set_editor_property("bIsGold", is_gold)
        except Exception as e: mark("SET_bIsGold_ERR",e)
        if is_gold:
            try: a.set_editor_property("GoldAmount", gold_amt)
            except Exception as e: mark("SET_GoldAmount_ERR",e)
        unreal.GameplayStatics.finish_spawning_actor(a,xf)
        return a
    except AttributeError:
        pass
    eas=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    a=eas.spawn_actor_from_class(pcls, loc, unreal.Rotator(0,0,0))
    try: a.set_editor_property("bIsGold", is_gold)
    except Exception as e: mark("SET_bIsGold_ERR2",e)
    if is_gold:
        try: a.set_editor_property("GoldAmount", gold_amt)
        except Exception as e: mark("SET_GoldAmount_ERR2",e)
    return a

base=pawn.get_actor_location()
g0=unreal.NexusAbilityUILibrary.get_run_inventory_gold(gw); p0=qty(pot_cls); f0=flask()
mark("START", "gold=%s potion=%s flask=%s"%(g0,p0,f0))

# GOLD pickup through the real reroute
gp=spawn_pickup(unreal.Vector(base.x+400,base.y+400,base.z+50), True, 25)
mark("GOLD_PICK", gp.get_name())
unreal.NexusAbilityUILibrary.handle_loot_pickup(gp, pawn)
g1=unreal.NexusAbilityUILibrary.get_run_inventory_gold(gw)
mark("AFTER_GOLD","gold=%s delta=%s destroyed=%s"%(g1,g1-g0,not unreal.SystemLibrary.is_valid(gp)))

# POTION pickup through the real reroute
pp=spawn_pickup(unreal.Vector(base.x+420,base.y+400,base.z+50), False)
mark("POT_PICK", pp.get_name())
unreal.NexusAbilityUILibrary.handle_loot_pickup(pp, pawn)
p1=qty(pot_cls)
mark("AFTER_POTION","potion=%s delta=%s destroyed=%s"%(p1,p1-p0,not unreal.SystemLibrary.is_valid(pp)))

# HUD gold text
try:
    hud=pc.get_editor_property("HudRef")
    if hud:
        try:
            gt=hud.get_editor_property("GoldText")
            mark("HUD_GOLDTEXT", (gt.get_text() if gt else "None"))
        except Exception as e: mark("HUD_GOLDTEXT_ERR",e)
    else: mark("HUD","HudRef None")
except Exception as e: mark("HUD_ERR",e)

# DRINK potion from inventory -> flask +1, stack -1
items=run_inv.find_items_of_class(pot_cls)
if items:
    run_inv.use_item(items[0])
    mark("AFTER_DRINK","flask=%s (was %s) potion=%s (was %s)"%(flask(),f0,qty(pot_cls),p1))
else:
    mark("DRINK","no potion")

# ISSUE2 flag + open/close mechanism
try:
    ci=unreal.get_default_object(unreal.load_class(None,"/Script/CommonInput.CommonInputSettings"))
    mark("ENH_INPUT", ci.get_editor_property("enable_enhanced_input_support"))
except Exception as e: mark("ENH_INPUT_ERR",e)

mark("DONE","ok")
