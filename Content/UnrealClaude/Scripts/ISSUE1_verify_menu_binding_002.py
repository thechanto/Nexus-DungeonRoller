import unreal

def L(m): unreal.log("ISSUE1 " + str(m))

ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = ues.get_game_world()
if not world:
    L("NO PIE WORLD - not playing"); raise SystemExit

pc = unreal.GameplayStatics.get_player_controller(world, 0)
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
L("pc=%s pawn=%s" % (pc.get_name() if pc else None, pawn.get_name() if pawn else None))

ps = pc.get_editor_property("player_state") if pc else None
L("player_state=%s" % (ps.get_name() if ps else None))

# Enumerate inventory components on the player state
run_inv = None; stash = None
if ps:
    comps = ps.get_components_by_class(unreal.NarrativeInventoryComponent)
    for c in comps:
        L("  found inv comp: %s" % c.get_name())
        if c.get_name() == "RunInventory": run_inv = c
        elif c.get_name() == "Stash": stash = c

# Which component does the plugin resolver return for the controller? (this is what the menu uses)
bound = unreal.InventoryFunctionLibrary.get_inventory_component_from_target(pc)
L("GetInventoryComponentFromTarget(pc) -> %s" % (bound.get_name() if bound else None))
L("  == RunInventory? %s   == Stash? %s" % (bound==run_inv, bound==stash))

# Grant test items to RunInventory
def cls(p): return unreal.load_class(None, p)
gold = cls("/Game/Inventory/Items/BP_Item_Gold.BP_Item_Gold_C")
potion = cls("/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")
axe = cls("/Game/Inventory/Items/BP_Item_Weapon_RustedAxe.BP_Item_Weapon_RustedAxe_C")
if run_inv:
    for c,q,nm in [(gold,137,"gold"),(potion,2,"potion"),(axe,1,"axe")]:
        if c:
            r = run_inv.try_add_item_from_class(c, q, False)
            L("  grant %s x%d -> AmountGiven=%s" % (nm, q, getattr(r,'amount_given', '?')))
    L("RunInventory now %d stack(s)" % len(run_inv.get_items()))
    L("Stash now %d stack(s)" % (len(stash.get_items()) if stash else -1))

# Open the menu via the InventoryUI component
uicomp = pc.get_component_by_class(unreal.NexusInventoryUIComponent) if pc else None
L("InventoryUI comp = %s" % (uicomp.get_name() if uicomp else None))
if uicomp:
    uicomp.toggle_inventory_menu()
    L("called ToggleInventoryMenu (open)")

# Find the open menu widget and read which inventory it bound
menu_cls = unreal.load_class(None, "/NarrativeInventory/NarrativeUI/W_NarrativeMenu_Inventory.W_NarrativeMenu_Inventory_C")
menus = unreal.WidgetBlueprintLibrary.get_all_widgets_of_class(world, menu_cls, False)
L("open menu instances: %d" % len(menus))
for m in menus:
    invw = None
    try: invw = m.get_editor_property("WBP_InventoryWidget")
    except Exception as e: L("  no WBP_InventoryWidget prop: %s" % e)
    if invw:
        bound_inv = None
        try: bound_inv = invw.get_editor_property("Inventory")
        except Exception as e: L("  no Inventory prop: %s" % e)
        L("  menu %s -> InventoryWidget.Inventory = %s (==Run? %s ==Stash? %s)" % (
            m.get_name(), bound_inv.get_name() if bound_inv else None,
            bound_inv==run_inv, bound_inv==stash))
        try:
            cached = invw.get_editor_property("CachedItems")
            L("  InventoryWidget.CachedItems count = %d" % len(cached))
        except Exception as e: L("  no CachedItems: %s" % e)
L("DONE")
