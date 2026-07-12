import unreal, os
out = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/T3DVerify"
os.makedirs(out, exist_ok=True)
at = unreal.AssetToolsHelpers.get_asset_tools()
at.export_assets(["/Game/GameplayAbilitySystem/Abilities/GA_Burden"], out)
print("exported")
