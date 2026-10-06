# Nexus: online co-op redesign (Solo Leveling style, Arc Raiders shape) — spec draft

Date: 2026-10-06. Status: draft for Niko's review; written by Claude (Fable 5.1) from Niko's answers
on 2026-10-06 in the Aide terminal. Nothing below is built. Decision recorded in Aide's vault on
2026-09-24: change the design toward a Solo Leveling-style online multiplayer co-op game with crossplay.

Niko's answers (2026-10-06): up to 5 players in a party, bigger counts later for events; PvE and
PvP; crossplay PC and console; lobby-based sessions like Arc Raiders.

## 1. Product (the PRD)

- **User problem:** Nexus today is a single-player UE 5.7 extraction prototype (one dungeon map, a
  boss, extraction banks loot, death forfeits it). Niko wants the same tension with friends: a party
  of up to 5 drops into a raid together, fights PvE, meets other parties (PvP), and extracts.
- **Success criterion (measurable):** two players on two PCs join one lobby, drop into
  `LV_Soul_Cave` together, kill the boss, and both extract with their loot banked, over the internet,
  in 5 of 5 attempts; no desync visible in a 10-minute session (positions, health, loot).
- **Launch note:** Nexus becomes a small-party extraction raider: lobby up, pick a raid, fight the
  dungeon and whoever else dropped in, get out alive or lose the run. First on PC, built from day
  one on a service layer that supports consoles, so crossplay is a platform contract later, not a
  rewrite.
- **Out of scope for this spec:** console builds and certification, large events (more than 5 per
  party), monetisation, anti-cheat, voice chat, progression rebalance for groups. Named so the
  framework fits them; each is its own spec.

## 2. Architecture

- **Components and flows:**
  1. **Service layer: Epic Online Services (EOS)** via the Online Subsystem EOS plugin: identity
     (Epic account on PC, platform accounts on console later), lobbies, sessions, P2P or
     dedicated-server connection, crossplay by design. Free. The one choice that makes "console
     later" possible without redoing lobbies.
  2. **Lobby:** `Lvl_MainMenu` gains Create / Join / Invite; a lobby of 1 to 5 players; ready-up;
     the host picks the raid map; the party travels together (seamless travel, listen server first).
  3. **Raid session:** one authoritative server (host-listen in slice 1, dedicated later), the
     current `LV_Soul_Cave` loop replicated: enemies, boss, chests, loot drops, extraction points.
  4. **PvP:** other parties can be placed into the same raid when the server has room (up to 2 or 3
     parties per instance in later slices); friendly fire off inside a party, on across parties.
  5. **Persistence:** stash and banked XP per player, saved server-side (EOS Player Data Storage or
     a tiny backend) so loot survives across sessions and devices.
- **Data:**
  - `PartyState` (replicated): members, ready flags, chosen map.
  - `RaidState` (replicated): phase (loading, live, extraction window, ended), timer, extraction
    points open/closed.
  - Player save: stash inventory, banked XP, level, cosmetic unlocks (JSON blob per EOS product
    user id).
- **Interfaces:** `UNexusOnlineSubsystem` (lobby create/join/leave, session travel),
  `ANexusGameMode` (server authority: spawn, loot, extraction, death), `ANexusGameState`
  (`RaidState`), `ANexusPlayerState` (party id, banked loot, PvP flags), replicated GAS attributes
  (already `UBasicAttributeSet`, `UCombatAttributeSet`: set replication flags, predict locally).
- **External dependencies:** EOS (free; developer portal account, product id, client id); Epic's
  Online Subsystem EOS plugin (in engine); later dedicated servers (Hathora or Epic's own hosting,
  roughly USD 20 to 50 a month for a small fleet while testing) and console partner programs
  (Sony PlayStation Partners, Xbox ID@Xbox: applications, dev kits, certification; months, not weeks).

## 3. Program design

- **File layout (Source/Nexus/):**
  ```
  Online/NexusOnlineSubsystem.h/.cpp     EOS login, lobby, session, travel
  Online/NexusPartyState.h/.cpp          replicated party
  Game/NexusGameMode.h/.cpp              server authority for the raid loop (extends the current mode)
  Game/NexusGameState.h/.cpp             RaidState
  Game/NexusPlayerState.h/.cpp           party id, banked loot, team for PvP
  Save/NexusCloudSave.h/.cpp             stash + XP to EOS Player Data Storage
  UI/W_Lobby, UI/W_PartyBar              lobby screen, party HUD (widget BPs; runtime trees in C++ per CLAUDE.md)
  docs/specs/2026-10-06-coop-redesign.md this file; docs/adr/ for the EOS and server decisions
  ```
- **Signatures:**
  ```cpp
  UCLASS() class UNexusOnlineSubsystem : public UGameInstanceSubsystem {
    void Login();                                   // EOS, platform account on consoles later
    void CreateLobby(int32 MaxMembers /*<=5*/);
    void JoinLobby(const FString& LobbyId);
    void SetReady(bool bReady);
    void StartRaid(FName Map);                      // host only: session + seamless travel
    FOnLobbyChanged OnLobbyChanged;
  };
  USTRUCT() struct FRaidState { ERaidPhase Phase; float SecondsLeft; TArray<FGuid> OpenExtractions; };
  UCLASS() class ANexusGameMode : public AGameModeBase {
    void OnPlayerExtracted(ANexusPlayerState* PS);   // bank loot server-side, then despawn
    void OnPlayerDied(ANexusPlayerState* PS);        // forfeit unbanked loot; spectate party
    bool CanDamage(const AActor* Source, const AActor* Target) const;  // party-aware friendly fire
  };
  ```
- **Call stack (slice 1):** Main menu Host -> `CreateLobby(2)` -> friend joins -> both Ready ->
  `StartRaid(LV_Soul_Cave)` -> seamless travel -> `ANexusGameMode::PostLogin` spawns both ->
  boss dies (server) -> loot replicated -> `OnPlayerExtracted` banks -> `ANexusCloudSave::Save`.
- **Tests planned:** (1) two PIE clients, one listen server: both see the boss die once (the
  double-loot-shower bug from BUGLOG must not return across the network); (2) extraction banks loot
  only for the extracting player; (3) death forfeits only the dead player's unbanked loot; (4) a
  client that disconnects mid-raid keeps its banked stash; (5) lobby ready-up refuses start with an
  unready member.
- **Failure modes:** host leaves (slice 1: raid ends, loot unbanked, say so; dedicated servers fix
  this later); EOS login fails (offline single-player stays available); replication of GAS
  attributes missing on a class (visible desync, test 1 catches it).

## 4. Slices

1. **Two players, one raid, LAN/listen.** Replicate the existing loop for 2 players over a listen
   server; lobby = a direct join by address. Usable: a friend plays the dungeon with Niko.
2. **EOS lobby and sessions.** Login, create/join/invite, ready-up, travel together; party of 5.
3. **Cloud stash.** Banked loot and XP survive sessions and machines.
4. **PvP: two parties per raid.** Team flags, friendly fire rules, extraction contest.
5. **Dedicated servers.** Host migration problem solved; bigger raids become possible.
6. **Consoles.** Platform accounts through EOS, partner programs, certification. Separate spec.

Estimate: slice 1 is two to three focused sessions (replication of what exists); slices 2 and 3
one week each; 4 and 5 depend on 1 to 3 holding up.

## 5. Decisions I am not confident about

1. **EOS vs Steamworks.** Steam is the easier PC launch and has lobbies, but Steam lobbies do not
   reach consoles; EOS does and is free. Recommendation: EOS for lobbies and identity, Steam only
   as a store and overlay later.
2. **Listen server vs dedicated from day one.** Listen is free and fast to build; dedicated costs
   money and setup but removes host advantage (PvP) and host-quit. Recommendation: listen for
   slices 1 to 4, dedicated at slice 5, before PvP goes to strangers.
3. **How much of the current single-player content survives.** The dungeon, boss, loot and
   extraction survive as the first raid. The potion/pause demo values and the single-player
   death screen need redesign for a party (spectate the party until extraction or wipe).
4. **Scope honesty.** Consoles for a solo developer mean dev kits and certification; realistic
   only after a PC version has players. The spec keeps the door open and does not promise it.

## 6. Review checklist (Niko)

- [ ] Success criterion is measurable and I agree with it.
- [ ] I read the signatures and file layout, not just the summary.
- [ ] Every "not confident" item has an answer or an ADR.
- [ ] Cost and time estimate are stated and acceptable.

Staging list for GitHub Desktop (commits are yours, per CLAUDE.md): `docs/specs/2026-10-06-coop-redesign.md`,
message "docs: co-op redesign spec draft (EOS lobbies, 5-player party, PvE+PvP, listen first)".
