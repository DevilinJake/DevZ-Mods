# DevZ Ammo Stacks

Changes how many rounds fit in one stack of loose ammo. Set one multiplier for everything, and
override any ammo type you want to treat differently. It only overrides one value per ammo
type (`count`), so it has no scripts and no performance cost.

Works with vanilla ammo and with any weapon mod: the scanner reads the ammo types from the game
and mod files themselves.

Want ammo to merge into your existing stacks automatically when you pick it up? See
[AutoStack](../AutoStack), a separate server-side mod.

## Default settings

- **2× stacks** for everything: 9×19 goes from 25 to 50, 12ga from 10 to 20, .22 from 50 to 100.
- **Single-round items stay at 1**: rockets, 40mm grenades, darts.
- **The ammo found in loot stays the same**, if you run `fix_loot.py` (see below). Only the
  stack size changes.
- **Weight per round doesn't change.** DayZ weighs ammo per round, so a stack of 50 weighs the
  same as two stacks of 25.

## The two settings files

**`settings.json`**: the global rules.

| Setting | Default | Meaning |
|---|---|---|
| `Multiplier` | `2.0` | Every stack is multiplied by this, then rounded |
| `LeaveSingleRoundAlone` | `true` | Ammo with a stack of 1 (rockets, 40mm…) stays at 1 |
| `MaxStack` | `1000` | Upper limit for any stack |

**`ammo.csv`**: one row per ammo type. Fill in `NewStack` to override the multiplier for that
type; leave it blank to use the multiplier.

| Column | Meaning |
|---|---|
| Class | The ammo type |
| Example | Its in-game name |
| Source | Vanilla, or the mod it comes from |
| CurrentStack | Rounds per stack now |
| **NewStack** | **Your size for this type. Blank = use the multiplier.** |

`Parent` and `RequiredAddon` are used to write the config; don't change them.

## Building it

You need **Python 3** and, to sign the mod for your server, **DayZ Tools** (free on Steam).

1. **Change `settings.json` and/or `ammo.csv`.**
2. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 AmmoStacks -Key C:\path\to\YourServer.biprivatekey
   ```
   This writes `src\config.cpp`, packs it and signs it. The mod is in `build\@DevZ_AmmoStacks`.
3. **Install it:** copy `@DevZ_AmmoStacks` to your server, its `keys\*.bikey` into the server's
   `keys\` folder, and add it to `-mod=`. Or add `-Install C:\path\to\DayZServer` to step 2.
4. **Keep loot the same** (recommended): with the server stopped, run
   ```powershell
   python AmmoStacks\fix_loot.py "C:\path\to\DayZServer\mpmissions\<your mission>\db\types.xml"
   ```

Players need the same copy of the mod, so publish it (Workshop) or give them the folder.

### Prefer editing the config directly?

`src\config.cpp` has one readable line per ammo type with the old size in a comment. Edit the
numbers and build with `-NoGenerate` so your edits aren't overwritten:

```cpp
class Ammo_9x19: Ammunition_Base { count = 50; };  // 9x19mm Rounds, was 25
```

## Why loot needs fixing

In `types.xml`, how much ammo spawns is set as a **percentage of a full stack** (`quantmin` and
`quantmax`). 9×19 spawns at 10–25% of a stack: 2–6 rounds. Double the stack and the same
percentages give 5–12 rounds, so there would be twice as much ammo in the world.

`fix_loot.py` scales those percentages by the same amount the stack changed, so 9×19 spawns at
5–12% of 50, which is still 2–6 rounds. It:

- saves a copy of your file as `types.xml.bak` the first time,
- writes each item's original percentages in a comment, so it's **safe to run again** after
  changing your stacks, and setting a stack back to its original size restores its loot too,
- only touches ammo types whose stack changed.

Want more ammo in the world instead? Don't run it.

## Using other weapon mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python AmmoStacks\scan_ammo.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeWeaponMod"
```

It rebuilds `ammo.csv` from what it finds and **keeps any `NewStack` you've already set**.

**The table in this repo includes Expansion's ammo.** A mod built from it requires Expansion to
be loaded. If your server doesn't run Expansion, re-run the scanner with just your own folders
first, and the Expansion rows drop out.

## Before you change stacks on a live server

Making stacks bigger is safe. Avoid making them **smaller** once players have ammo: piles
already bigger than the new size haven't been tested and may behave oddly. Set your stacks
before launch, or only increase them.
