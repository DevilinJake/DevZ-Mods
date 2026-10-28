# DevZ Repair To Pristine

Repair kits no longer stop at **Worn**. Keep repairing and the item comes back to **Pristine**,
just like tents and base building already do in the game.

Each repair still moves the item up one condition level and uses up some of the kit, so going
from Worn to Pristine costs one more repair's worth of kit.

- **Players need it too** (it's a normal mod, not a server-only one). The game decides on the
  player's side whether to offer "repair" on a Worn item.
- **Works with kits from any mod.** A scanner lists every repair kit in the game and your mods,
  and you pick which ones repair to Pristine.

## Why a mod is needed

When you repair something, the game asks the damaged item "can you be repaired to Pristine?".
Almost every item answers no; only tents, fences, watchtowers, shelters and the territory flag
say yes. The kit is never asked, and there's no config setting for it. This mod remembers which
kit is being used while a repair is checked or done, and says yes when that kit is on the list.

## Default kits

Every vanilla kit is on, so everything repairable can go back to Pristine:

| Kit | Class | Repairs |
|---|---|---|
| Gun Cleaning Kit | `WeaponCleaningKit` | Guns, magazines, optics |
| Sewing Kit | `SewingKit` | Cloth clothing and bags |
| Leather Sewing Kit | `LeatherSewingKit` | Leather clothing and bags |
| Duct Tape | `DuctTape` | Most clothing, bags and many tools |
| Sharpening Stone | `Whetstone` | Knives, axes and blades |
| Epoxy Putty | `EpoxyPutty` | Helmets, hard plastics, some gun parts |
| Electrical Repair Kit | `ElectronicRepairKit` | Radios, lights, batteries |
| Tire Repair Kit | `TireRepairKit` | Vehicle wheels |
| Blowtorch | `Blowtorch` | Metal vehicle and base parts |

Kits built on a listed kit count too. Listing a kit from a mod you don't run is harmless.

**Ruined items stay the game's way.** The mod only changes the Worn → Pristine step; it doesn't
make Ruined items repairable.

**ZenSkills:** its Crafting perk "chance of repairing damaged gear back to Pristine" still
works: when a repair brings an item up to Worn, it can jump straight to Pristine, saving the last
repair. Take kits off the list if you want Pristine to be only a perk reward.

## Changing the list

The list is `kits.csv`. Open it in Excel or any text editor. Put `Yes` in the **Pristine**
column for each kit that should repair to Pristine, and leave it empty for kits that should stop
at Worn as normal.

| Column | Meaning |
|---|---|
| `Pristine` | `Yes` = repairs to Pristine. Empty = stops at Worn (the game's normal rule) |
| `Class` | The kit's class name (what the game and `types.xml` call it) |
| `Example` | The name players see |
| `KitType` | The game's kit number. Items list which kit numbers can repair them |
| `Repairs` | How many items (in the game and scanned mods) that kit number can repair |
| `Source` | `Vanilla`, or the mod it comes from |

Then build the mod (below).

### Prefer editing the config directly?

The build writes the list into `src\config.cpp`, one class name per line:

```cpp
class CfgDevZ_RepairToPristine
{
	kits[] =
	{
		"WeaponCleaningKit",
		"SewingKit"
	};
};
```

You can edit that by hand and build with `-NoGenerate` so it isn't overwritten.

## Adding kits from other mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python RepairToPristine\scan_kits.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeMod"
```

It adds every repair kit from those mods to `kits.csv` (new ones start as `Yes`) and **keeps
the choices you've already made**. For example, Zen's Swiss Army Knife scissors work as a
sewing kit and show up here.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 RepairToPristine -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_RepairToPristine`** into your server folder and its key into `keys\`.
3. **Add it to `-mod=`** in your start line, e.g. `-mod=@CF;...;@DevZ_RepairToPristine;`
4. **Players add it in their launcher** like any other mod.

Changing the list later means rebuilding the mod, so players download the new version.

## Works with other mods

It adds to the game's repair code instead of replacing it, so mods that also change repairing
(like ZenSkills' kit-saving perks) keep working.

## Your own signing key

Servers with `verifySignatures=2` only load mods signed with a key they trust. Make one with
DayZ Tools: `Bin\DsUtils\DSCreateKey.exe YourServer`. Keep the `.biprivatekey` private and
share only the `.bikey`.
