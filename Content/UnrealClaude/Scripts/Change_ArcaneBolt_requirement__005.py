# @UnrealClaude Script
# @Name: FixArcaneBoltStatType
# @Description: Arcane Bolt's single requirement was authored as Strength; change its StatType to Intelligence (value stays 20) and save.
import unreal

ASSET = "/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_ArcaneBolt"
ST_PROP = "StatType_4_9B7A4DEE48E48A7CD9AD51827383BED4"
SV_PROP = "StatValue_5_8729D6CB40E4E437DB60BCB44F8C6FEF"

cls = unreal.load_object(None, ASSET + ".DA_Ability_Mage_ArcaneBolt_C")
cdo = unreal.get_default_object(cls)

data = cdo.get_editor_property("AbilityData")
reqs_prop = None
import re
m = re.search(r'\b(Requirements_\d+_[0-9A-Fa-f]{32})=', data.export_text())
if not m:
    raise RuntimeError("Requirements member not found")
reqs_prop = m.group(1)

reqs = data.get_editor_property(reqs_prop)
if len(reqs) != 1:
    raise RuntimeError("Expected exactly 1 requirement, found %d - aborting" % len(reqs))

req = reqs[0]
print("before: %s" % req.export_text())
req.set_editor_property(ST_PROP, unreal.NexusStatType.INTELLIGENCE)
reqs[0] = req
data.set_editor_property(reqs_prop, reqs)
cdo.set_editor_property("AbilityData", data)

check = cdo.get_editor_property("AbilityData").get_editor_property(reqs_prop)[0]
print("after:  %s" % check.export_text())
if not unreal.EditorAssetLibrary.save_asset(ASSET, only_if_is_dirty=False):
    raise RuntimeError("Failed to save " + ASSET)
print("DONE - ArcaneBolt now requires Intelligence %s" % check.get_editor_property(SV_PROP))
