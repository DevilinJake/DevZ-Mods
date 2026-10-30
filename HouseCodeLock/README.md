# DevZ House Code Lock

Fit the **DayZ Expansion code lock** to rebuilt vanilla house doors. Expansion's code lock
already works on its own walls, vanilla fence gates and tents, but not on house doors yet.
This mod fills that gap until Expansion supports house doors itself.

- **Needs DayZ Expansion** (the Bundle, or Base Building). Load this mod after it.
- **Players need it too** (it's a normal mod, not a server-only one).
- **Nothing in Expansion is changed or repacked.** The mod only adds to it from its own files,
  and Expansion's code lock on gates, walls, tents and safes works exactly as before.

## What it does

Rebuild a house door with the vanilla rebuilding (bricks, mortar, trowel), close it and:

| Do this | What happens |
|---|---|
| Code lock in hands, look at the closed door | **Attach**. The lock goes in the same spot as the vanilla digital code lock |
| Look at the lock | **Set code** opens Expansion's keypad. With the door shut it locks straight away |
| Look at an unlocked lock | **Close & lock** (shuts the door if it's open), or **Change code** |
| Look at a locked lock | **Enter code**, or **Unlock** if you've entered the code before |

- A locked door can't be opened from either side, the same as an Expansion gate.
- A lockpick can't lock or unlock a door with the code lock fitted.
- One lock per door. It can't go next to a vanilla lock on the same door, and only goes on
  while the door is shut (the vanilla rules for its own locks).
- A ruined lock (shot or blown up) no longer holds the door, like the vanilla locks.
- The lock can't be taken off while it has a code. Set the code to all zeroes to clear it,
  the same as on a gate.
- **Codes and lock states survive restarts.**

Expansion's own BaseBuilding settings apply as they do on gates: code length, remembering
players who know the code (`RememberCode`), territory members (`AuthenticateCodeLockIfTerritoryMember`),
damage for a wrong code (`DoDamageWhenEnterWrongCodeLock`) and the code lock raid logs.

**Not covered:** Expansion's own lock-raiding tools (`ExpansionActionDestroyLock`) only know
items, so they don't work on a house lock. To raid a locked house door, destroy the door or the
lock (with your server's own raid rules).

**Territory lock limits don't count house locks.** Expansion counts code locks per territory
(`MaxCodeLocksOn*PerTerritory`) when one goes on an item; a house isn't one, so house locks are
neither limited nor counted.

**Harmless log line:** on every restart, Expansion prints
`ExpansionCodeLock::DeferredInit - removing code from unattached ...` for each house lock. The
mod puts the code, lock state and known players straight back afterwards.

## Looks

The lock is drawn with Expansion's model in the vanilla code lock's spot on the door, so it
can sit a little off. The light is red when unlocked and green when locked.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 HouseCodeLock -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_HouseCodeLock`** into your server folder, add it to `-mod=` after
   Expansion, and copy its key from `keys\` into your server's `keys\`.
3. Players get the same copy (Workshop or your server pack).

## Settings

None of its own. It follows your Expansion BaseBuilding settings.

## How it works

Expansion's lock code expects the lock to sit on an item (fence, wall, tent, crate): the attach
action, the keypad and the lock checks all look for an item. A house is a building, not an
item, so none of it fires there. But the code, the locked state and the list of players who
know the code are stored **on the lock itself**, so the mod points Expansion's own keypad and
server messages at the lock instead of at its parent. Everything Expansion checks on the server
(the code, wrong-code damage, known players, logs) runs unchanged.

Code: `src\scripts\4_World\DevZ_HouseCodeLock\`

| File | What |
|---|---|
| `config.cpp` | Adds the vanilla digital code lock's 20 door spots (`Att_CodeLock1_Wood` ... `Att_CodeLock10_Metal`) to `ExpansionCodeLock`'s `inventorySlot[]` |
| `DevZ_HouseCodeLock.c` | Finds which door a lock spot belongs to, and the lock on a door |
| `DevZ_ActionAttachCodeLockToHouse.c` | "Attach": the vanilla attach-to-door action, limited to house doors |
| `DevZ_ActionHouseCodeLock.c`, `DevZ_ActionHouseChangeCode.c` | Set code, Close & lock, Enter code, Unlock, Change code: the same choices as Expansion's gate actions, sent to the lock |
| `ExpansionCodeLock.c` | On a house the lock counts as "open" while its door is, "Close & lock" shuts the door, the light shows the lock state, and the code is kept over a restart (Expansion's start-up check clears the code of a lock that isn't on an item) |
| `BuildingBase.c` | A door with a locked code lock won't open, and the vanilla door-lock rules apply when the lock is fitted |

**For the Expansion team:** if you add house doors yourselves, the two spots that need it are
the parent lookups (`ItemBase.Cast(GetHierarchyParent())` in `ExpansionCodeLock.DeferredInit`
and `UpdateVisuals_Deferred`, and `ItemBase.Cast(target...)` in the code lock actions) and a
`BuildingBase.CanDoorBeOpened` check. Feel free to take anything from here (MIT).
