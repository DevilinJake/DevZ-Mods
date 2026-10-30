# DevZ Battery Life

Makes batteries hold more energy, so every flashlight, headtorch, NVG, red dot, radio and car
battery lasts longer. Set one multiplier for batteries (and one for gas canisters, if you want),
and change any single battery you want to treat differently. It only overrides one value per
battery (`energyStorageMax`), so it has no scripts and no performance cost.

Works with vanilla batteries and with any mod's: the scanner reads them from the game and mod
files themselves.

## How battery life works

A battery holds `energyStorageMax` units of energy. A device uses its own
`energyUsagePerSecond` while it's switched on. So **2× the capacity = 2× as long** on every
device that runs on that battery, without touching the devices.

| Device (vanilla) | Uses per second | On one 9V, vanilla | With 2× |
|---|---|---|---|
| Flashlight, headtorch, NVGs, rangefinder, GPS, weapon light | 0.01 | 83 min | 2 h 47 min |
| Red dots and lit scopes, radio, megaphone, defibrillator | 0.02 | 42 min | 83 min |

A car battery holds 500 (truck 1500). With the engine off, headlights use 0.25 per second
(33 minutes, 67 with 2×), and each engine start uses 15.

Gas canisters work the same way for gas lamps, stoves and the blowtorch.

## Default settings

- **Batteries hold 2× as much**: 9V, car and truck battery.
- **Gas canisters stay vanilla** (`GasMultiplier` 1.0).
- Every battery and canister can be set on its own in `batteries.csv`.

New batteries spawn as full as before (`energyAtSpawn` is raised by the same multiplier). A
battery charger and a running engine add the same charge per second as before, so charging a
battery from empty to full takes longer by the same factor.

## The two settings files

**`settings.json`**: the global multipliers.

| Setting | Default | Meaning |
|---|---|---|
| `BatteryMultiplier` | `2.0` | Every battery holds this many times as much |
| `GasMultiplier` | `1.0` | Every gas canister holds this many times as much |

Use `1.0` for vanilla, or less than 1 to make things run out faster.

**`batteries.csv`**: one row per battery or canister. Fill in `Multiplier` to give that row its
own value; leave it blank to use the global one.

| Column | Meaning |
|---|---|
| Type | Battery or Gas |
| Class | The item's class |
| Example | Its in-game name |
| Source | Vanilla, or the mod it comes from |
| CurrentCapacity | Its `energyStorageMax` now |
| CurrentAtSpawn | Its `energyAtSpawn` now (how full it spawns) |
| **Multiplier** | **Your multiplier for this row. Blank = use the one in settings.json.** |

`Parent` and `RequiredAddon` are used to write the config; don't change them.

## Building it

You need **Python 3** and, to sign the mod for your server, **DayZ Tools** (free on Steam).

1. **Change `settings.json` and/or `batteries.csv`.**
2. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 BatteryLife -Key C:\path\to\YourServer.biprivatekey
   ```
   The mod is in `build\@DevZ_BatteryLife`.
3. **Install it:** copy `@DevZ_BatteryLife` to your server, its `keys\*.bikey` into the
   server's `keys\` folder, and add it to `-mod=`. Or add `-Install C:\path\to\DayZServer` to
   step 2.

Players need the same copy of the mod, so publish it (Workshop) or give them the folder.

### Prefer editing the config directly?

`src\config.cpp` has one readable line per battery with the old value and multiplier in a
comment. Edit the numbers and build with `-NoGenerate` so your edits aren't overwritten:

```cpp
class Battery9V: Inventory_Base { class EnergyManager { energyStorageMax = 100; energyAtSpawn = 100; }; };  // 9V Battery, was 50 (x2)
```

## Using other mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python BatteryLife\scan_batteries.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeMod"
```

It rebuilds `batteries.csv` from what it finds and **keeps any `Multiplier` you've already set**.
It lists every item that stores energy for other items to use (batteries and gas canisters),
not devices with a charge of their own such as the chainsaw.

**The table in this repo lists vanilla items only**, so the mod runs on any server. Rows you
add by scanning a mod make the build require that mod, so only scan mods your server runs.

## Changing it on a live server

Safe at any time. The game saves how much energy each battery holds, not a percentage, so
batteries already in the world keep their charge: a full 9V shows half full after you switch to
2×, and can be charged up the rest of the way. New batteries spawn full.
