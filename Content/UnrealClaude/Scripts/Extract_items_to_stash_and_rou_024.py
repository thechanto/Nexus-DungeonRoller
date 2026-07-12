import unreal
M = "[ITEMTEST]"
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
ps = unreal.GameplayStatics.get_player_state(w, 0)
comps = ps.get_components_by_class(unreal.NarrativeInventoryComponent)
run_inv = next(c for c in comps if c.get_name() == "RunInventory")
stash = next(c for c in comps if c.get_name() == "Stash")

def dump(tag, inv):
    items = [f"{it.get_class().get_name()}x{it.get_editor_property('quantity')}" for it in inv.get_items()]
    unreal.log(f"{M} {tag}: {items if items else 'EMPTY'}")

dump("stash_before", stash)
dump("run_before", run_inv)
unreal.NexusAbilityUILibrary.extract_run_inventory_to_stash(ps)
dump("run_after_extract", run_inv)
dump("stash_after_extract", stash)

# round-trip: wipe stash in memory, reload from the slot the extract just saved
for it in list(stash.get_items()):
    stash.remove_item(it)
dump("stash_wiped", stash)
unreal.NexusAbilityUILibrary.load_stash(ps)
dump("stash_reloaded", stash)
