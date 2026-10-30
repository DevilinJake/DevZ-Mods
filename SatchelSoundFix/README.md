# DevZ Satchel Sound Fix

Fixes a bug in **DayZ Expansion** (Base Building): after a satchel charge explodes, its arming
beep keeps going at the crater, faster and faster, for 5 to 15 minutes, fading with distance.
With this mod the beeping stops with the bang.

- **Players need it too** (it's a normal mod, not a server-only one): the beep is played on
  each player's own game, so the fix has to be there.
- **Needs Expansion Base Building.** Without it the mod compiles to nothing and does no harm.
  Load it **after** Expansion.
- **Changes nothing else.** No items, no settings, no Expansion files: it's a small script
  override, the same way every Expansion add-on works.

## The bug

Expansion's satchel (`ExpansionSatchel`, built on `ExpansionExplosive`) beeps while it's armed.
Each player's game runs a tiny "every frame" job for an armed charge; the job beeps whenever a
counter reaches the seconds left on the fuse. When the charge explodes, the game plays the bang,
but **nothing stops that per-frame job**. Worse, the per-frame job itself keeps the charge
alive on the player's side after the server has deleted it. And by then the seconds left are 0,
so the counter check can never wait: a fresh beep starts **every single frame**. That's the
squeal you hear at the crater until the game eventually drops the object.

## The fix

Two small overrides of `ExpansionExplosive`, in
`src\scripts\4_World\DevZ_SatchelSoundFix\ExpansionExplosive.c`:

1. When the explosion arrives on a player's game, Expansion's own code runs first (the bang,
   the particles); then the per-frame beep job is removed and any held sound is stopped.
2. The beep itself refuses to play once the charge has exploded or is no longer armed, and
   removes the job if it's somehow still there. A time of 0 seconds (the last moment before the
   explosion arrives) is treated as 1 second, so you hear the same quick beep as the last second
   of the countdown instead of a beep every frame.

The whole file is wrapped in `#ifdef EXPANSIONMODBASEBUILDING`, Expansion's own marker, so
without Expansion Base Building nothing is compiled. The comment at the top of the file
describes the bug in Expansion's terms, so its team can lift the fix if they want it.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 SatchelSoundFix -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_SatchelSoundFix`** into your server folder, add it to `-mod=` **after
   Expansion**, and copy its key from `keys\` into your server's `keys\`.
3. Players get the same copy (Workshop or your server pack).

## Settings

None. There's nothing for admins to set.

## Testing it

Plant a satchel charge on something, step back and let it blow. The beeping should stop the
moment it explodes, both for the player who planted it and for anyone standing nearby. Without
the mod, the beep carries on and speeds up into a squeal.
