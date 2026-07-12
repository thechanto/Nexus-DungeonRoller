import unreal

def mark(k,v): unreal.log("DROPVERIFY|%s|%s" % (k, v))

les=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.is_in_play_in_editor():
    mark("FATAL","not in PIE"); raise SystemExit

gw=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pawn=unreal.GameplayStatics.get_player_pawn(gw,0)
pc=unreal.GameplayStatics.get_player_controller(gw,0)
mark("PAWN", pawn.get_name() if pawn else "None")
mark("PAWN_CLASS", pawn.get_class().get_name() if pawn else "None")

# locate RunInventory component (on player state)
ps=pc.get_editor_property("player_state") if pc else None
run_inv=None
for holder in [ps, pawn, pc]:
    if not holder: continue
    for c in holder.get_components_by_class(unreal.NarrativeInventoryComponent):
        if 'RunInventory' in c.get_name(): run_inv=c
    if run_inv: break
mark("RUN_INV", run_inv.get_name() if run_inv else "None")

def potion_qty():
    if not run_inv: return -1
    cls=unreal.load_class(None,"/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")
    tot=0
    for it in run_inv.find_items_of_class(cls):
        if it: tot+=it.get_quantity()
    return tot

def flask():
    try: return pawn.get_editor_property("HealthPotionCount ")
    except Exception as e:
        try: return pawn.get_editor_property("HealthPotionCount")
        except Exception as e2: return "ERR:%s"%e2

gold0=unreal.NexusAbilityUILibrary.get_run_inventory_gold(gw)
pot0=potion_qty()
flask0=flask()
mark("START_GOLD", gold0); mark("START_POTION", pot0); mark("START_FLASK", flask0)

# spawn a gold pickup away from player, set props, run the real HandleLootPickup
loc=pawn.get_actor_location(); loc=unreal.Vector(loc.x+400,loc.y+400,loc.z+50)
pcls=unreal.load_class(None,"/Game/Loot/BP_LootPickup.BP_LootPickup_C")
mark("PICKUP_CLASS", "ok" if pcls else "None")
xf=unreal.Transform(); xf.translation=loc
gold_pick=unreal.GameplayStatics.begin_deferred_actor_spawn_from_class(gw,pcls,xf,unreal.SpawnActorCollisionHandlingMethod.ALWAYS_SPAWN)
try: gold_pick.set_editor_property("bIsGold", True)
except Exception as e: mark("SET_bIsGold_ERR", e)
try: gold_pick.set_editor_property("GoldAmount", 25)
except Exception as e: mark("SET_GoldAmount_ERR", e)
unreal.GameplayStatics.finish_spawning_actor(gold_pick, xf)
mark("GOLD_PICK", gold_pick.get_name())
unreal.NexusAbilityUILibrary.handle_loot_pickup(gold_pick, pawn)
mark("GOLD_PICK_DESTROYED", not unreal.SystemLibrary.is_valid(gold_pick))

gold1=unreal.NexusAbilityUILibrary.get_run_inventory_gold(gw)
mark("AFTER_GOLD_GOLD", gold1)
mark("GOLD_DELTA", gold1-gold0)

# potion pickup
loc2=unreal.Vector(loc.x+50,loc.y,loc.z)
xf2=unreal.Transform(); xf2.translation=loc2
pot_pick=unreal.GameplayStatics.begin_deferred_actor_spawn_from_class(gw,pcls,xf2,unreal.SpawnActorCollisionHandlingMethod.ALWAYS_SPAWN)
pot_pick.set_editor_property("bIsGold", False)
unreal.GameplayStatics.finish_spawning_actor(pot_pick, xf2)
unreal.NexusAbilityUILibrary.handle_loot_pickup(pot_pick, pawn)
pot1=potion_qty()
mark("AFTER_POTION_POTION", pot1); mark("POTION_DELTA", pot1-pot0)

# HUD gold text
try:
    hud=pc.get_editor_property("HudRef")
    if hud:
        gt=None
        for w in [hud]:
            gt=hud.get_editor_property("GoldText") if hud else None
        # GoldText added programmatically; try find widget
        found=unreal.WidgetLibrary.get_all_widgets_with_name if hasattr(unreal.WidgetLibrary,'get_all_widgets_with_name') else None
        try:
            txt=hud.get_editor_property("GoldText")
            mark("HUD_GOLDTEXT", txt.get_text() if txt else "None")
        except Exception as e:
            mark("HUD_GOLDTEXT_ERR", e)
    else:
        mark("HUD","HudRef None")
except Exception as e:
    mark("HUD_ERR", e)

# drink potion from inventory -> flask +1
cls=unreal.load_class(None,"/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")
items=run_inv.find_items_of_class(cls)
if items:
    potion_item=items[0]
    run_inv.use_item(potion_item)
    flask1=flask()
    pot2=potion_qty()
    mark("AFTER_DRINK_FLASK", flask1); mark("FLASK_DELTA_STR", "%s->%s"%(flask0,flask1))
    mark("AFTER_DRINK_POTION", pot2); mark("DRINK_POTION_DELTA", pot2-pot1)
else:
    mark("DRINK","no potion item found")

# ISSUE2: enhanced input support flag now active?
try:
    ci=unreal.get_default_object(unreal.load_class(None,"/Script/CommonInput.CommonInputSettings"))
    mark("COMMONINPUT_ENHANCED", ci.get_editor_property("enable_enhanced_input_support"))
except Exception as e:
    mark("COMMONINPUT_ERR", e)

mark("DONE","ok")
