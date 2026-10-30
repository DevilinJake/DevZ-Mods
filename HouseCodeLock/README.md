# DevZ Expansion House CodeLock

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
| Code lock in hands, look at the closed door | **Attach**. A keypad appears on each side of the door |
| Look at the door | **Set Code** opens Expansion's keypad. With the door shut it locks straight away |
| Door locked, you know the code | **Open locked door**: one step, the same as an Expansion gate. Opening unlocks it |
| Door locked, you don't | **Enter Code** (keypad), or **Unlock** once you've entered it before |
| Door unlocked, code set | **Lock** (shuts the door first if it's open), or **Change Code** |

It works exactly like the Expansion code lock on a vanilla fence gate:

- While it's **locked**, only players who know the code can open the door (anyone who has
  entered the right code, or a territory member when Expansion is set up that way). They just
  look at the door and open it; that unlocks the lock (light goes red).
- While it's **unlocked**, anyone can use the door. Lock it again with **Lock**, or (with
  Expansion's `CodelockActionsAnywhere` off, its default) by closing the door, which then reads
  **Lock**.
- The lock actions come from the door itself, so they work from both sides. With
  `CodelockActionsAnywhere` off they only show while looking at one of the keypads, like the lock
  selection on a gate; on, from anywhere on the door.
- A lockpick can't lock or unlock a door with the code lock fitted.
- One lock per door. It can't go next to a vanilla lock on the same door, and only goes on
  while the door is shut (the vanilla rules for its own locks).
- A ruined lock (shot or blown up) no longer holds the door, like the vanilla locks.
- The lock can't be taken off while it has a code. **Change Code** to all zeroes clears it,
  the same as on a gate; then it can be taken off.
- **Codes and lock states survive restarts.**

Expansion's own BaseBuilding settings apply as they do on gates: code length, remembering
players who know the code (`RememberCode`), territory members (`AuthenticateCodeLockIfTerritoryMember`),
lock actions anywhere on the door (`CodelockActionsAnywhere`), the keypad opening right after
attaching (client setting), damage for a wrong code (`DoDamageWhenEnterWrongCodeLock`) and the
code lock raid logs.

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

Two keypads, one on each side of the door, drawn with Expansion's lock model. The light is red
when unlocked and green when locked.

Why two: the door's lock spot was made for the vanilla digital code lock, and it points
Expansion's model the wrong way (keypad into the door), with nothing to use from inside. So the
real lock sits hidden in that spot and the mod draws two keypads as children of it: the outside
one is the model turned round, the inside one is moved through the door onto its inside face.
They're client-side only (never saved or sent over the network) and follow the lock, so they
move with the door. A short ray from the lock finds the door and measures its thickness; if that
fails (the door was open at the time), the fallbacks in `config.cpp` are used:
`devz_doorThickness` (m), `devz_lockDepth` (m) and `devz_keypadDir` (the keypad's direction in
the lock model's own axes). Tweak those if the keypads sit off on your map's doors.

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
| `config.cpp` | Adds the vanilla digital code lock's 20 door spots (`Att_CodeLock1_Wood` ... `Att_CodeLock10_Metal`) to `ExpansionCodeLock`'s `inventorySlot[]`; the keypad visual class and its fallback measurements |
| `DevZ_HouseCodeLock.c` | Finds which door a lock spot belongs to, the lock on a door, and the door the player is looking at (door or lock) |
| `DevZ_ActionAttachCodeLockToHouse.c` | "Attach": the vanilla attach-to-door action, limited to house doors |
| `DevZ_ActionHouseCodeLock.c`, `DevZ_ActionHouseChangeCode.c`, `PlayerBase.c` | Set Code, Lock, Enter Code, Unlock, Change Code: the same choices as Expansion's gate actions, sent to the lock. Player actions on the door, listed after Open / Close |
| `ActionOpenDoors.c`, `ActionCloseDoors.c` | "Open locked door" for players who know the code; "Close" locks when `CodelockActionsAnywhere` is off |
| `ExpansionCodeLock.c` | On a house the lock counts as "open" while its door is, "Lock" shuts the door, the code is kept over a restart (Expansion's start-up check clears the code of a lock that isn't on an item), and the two keypads are placed and lit |
| `DevZ_HouseCodeLockVisual.c` | The keypad object: no actions, not takeable, invisible to the cursor and the vicinity |
| `BuildingBase.c` | A locked door opens only for a player who knows the code, opening unlocks it, and the vanilla door-lock rules apply when the lock is fitted |

**For the Expansion team:** if you add house doors yourselves, the spots that need it are the
parent lookups (`ItemBase.Cast(GetHierarchyParent())` in `ExpansionCodeLock.DeferredInit` and
`UpdateVisuals_Deferred`, and `ItemBase.Cast(target...)` in the code lock actions), a
`BuildingBase.CanDoorBeOpened` check with `IsKnownUser`, and a two-sided model placement for the
vanilla door slots (they face the model into the door). Feel free to take anything from here (MIT).
