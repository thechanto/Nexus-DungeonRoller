# BUGLOG — resolved bugs (what · root cause · fix · date)

- Snipe-kill XP granted nothing · AddXP sat inside the token ForEach LoopBody, sniped enemies reserve no token · moved XP to the ForEach Completed pin (Completed→Cast→AddXP→SetDead→SpawnLootDrop), fires once unconditionally; readback-verified (snipe PIE condition not reproducible by user, so code-verified) · 2026-07-17
- GA_Death left player input live ~2s post-death · Disable Input node had zero exec connections (orphaned) · spliced Cast To PlayerController→DisableInput→Delay + wired its PlayerController pin; PIE-confirmed input dead immediately · 2026-07-18
- Enemy damage-after-death, WEAPON path only · weapon hit-scan is a looping timer cleared solely by the HitScanEnd anim-notify, which a death-interrupted montage skips · death chain now calls WeaponActor→HitScanEnd guarded by IsValid(WeaponActor); ability + AOE-field paths remain OPEN (CLAUDE.md §6 item 1) · 2026-07-18
