# DevZ Combine No Split

Combine part-used bandages, repair kits and gas mask filters into one, the same way you combine
rags or ammo. Drag a bandage with 2 uses onto one with 3 uses and you get a full bandage (4 uses)
plus one with 1 use left. Kits work the same way with their percentage: a 60% sewing kit onto
another 60% one gives you a 100% kit and a 20% kit.

They can **never be split**, so nobody can turn one 4-use bandage into four 1-use bandages.

- **Players need it too** (it's a normal mod, not a server-only one). The game decides on the
  player's side whether to offer "combine" when you drag one item onto another.
- **Works with items from any mod.** A scanner lists every item with uses or a percentage in the
  game and your mods, and you pick which ones combine.

## Why a mod is needed

In the game's code every item has two separate switches: *can be combined* and *can be split*.
Both are set from one config setting, `canBeSplit`. Bandages and repair kits have it off, so
they can't do either. Turning `canBeSplit` on would let them combine, but also let players split
them. This mod sets the two switches separately: combine on, split off.

## Default items

| Item | Class | Uses |
|---|---|---|
| Bandage | `BandageDressing` | 4 uses |
| Duct Tape | `DuctTape` | % |
| Electrical Repair Kit | `ElectronicRepairKit` | % |
| Epoxy Putty | `EpoxyPutty` | % |
| Gas Mask Filter | `GasMask_Filter` | % |
| Improvised Gas Mask Filter | `GasMask_Filter_Improvised` | % |
| Leather Sewing Kit | `LeatherSewingKit` | % |
| Sewing Kit | `SewingKit` | % |
| Tire Repair Kit | `TireRepairKit` | % |
| Gun Cleaning Kit | `WeaponCleaningKit` | % |
| Sharpening Stone | `Whetstone` | % |

To add items from your own mods (for example a drug mod's part-used bottles), run the scanner
below. Listing an item from a mod you don't run is harmless: the mod just never finds it.

When two items combine, the full one gets the average condition of the two, weighted by how much
each had (the game's normal rule). Ruined items can't be combined.

## Changing the list

The list is `items.csv`. Open it in Excel or any text editor. Put `Yes` in the **Combine**
column for each item you want, and leave it empty for the rest.

| Column | Meaning |
|---|---|
| `Combine` | `Yes` = combine but never split. Empty = the game's normal behaviour |
| `Class` | The item's class name (what the game and `types.xml` call it) |
| `Example` | The name players see |
| `MaxQuantity` | Full amount: uses (bandage = 4) or a percentage-style amount (kits = 100) |
| `ShownAs` | `bar` = shown as a bar or %, `count` = shown as a number |
| `SplitsNow` | `Yes` = the game already combines **and splits** this item (rags, nails, logs) |
| `Source` | `Vanilla`, or the mod it comes from |

Then build the mod (below). Items set to `Yes` show at the top of the file.

**About `SplitsNow` items:** setting one of these to `Yes` stops it splitting. That's useful for
a modded bandage that some mod made splittable. Don't do it for building materials, money or
anything players need to split into piles (nails, planks, logs, banknotes).

### Prefer editing the config directly?

The build writes the list into `src\config.cpp`, one class name per line:

```cpp
class CfgDevZ_CombineNoSplit
{
	items[] =
	{
		"BandageDressing",
		"SewingKit"
	};
};
```

You can edit that by hand and build with `-NoGenerate` so it isn't overwritten.

## Adding items from other mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python CombineNoSplit\scan_items.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeMod"
```

It adds every item with uses or a percentage from those mods to `items.csv` and **keeps the
choices you've already made**. Then set the new ones you want to `Yes` and build.

The scanner leaves out things that use "quantity" for something else: ammo, food, drinks,
liquids, clothing, vehicles and cooking pots. It also leaves out the few vanilla items that
already combine without splitting (matchbox, disinfectant, iodine, plant material).

Mods don't need to load before this one: the list is only read when an item is created.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 CombineNoSplit -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_CombineNoSplit`** into your server folder and its key into `keys\`.
3. **Add it to `-mod=`** in your start line, e.g. `-mod=@CF;...;@DevZ_CombineNoSplit;`
4. **Players add it in their launcher** like any other mod.

Changing the list later means rebuilding the mod, so players download the new version.

## Your own signing key

Servers with `verifySignatures=2` only load mods signed with a key they trust. Make one with
DayZ Tools: `Bin\DsUtils\DSCreateKey.exe YourServer`. Keep the `.biprivatekey` private and
share only the `.bikey`.
