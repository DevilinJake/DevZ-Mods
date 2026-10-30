# DevZ Weapon & Suppressor Durability

Makes weapons, suppressors and other muzzle attachments wear out slower (or faster). Set one
multiplier for weapons and one for suppressors, and change any single weapon or attachment you
want to treat differently. It only overrides one value per item (`barrelArmor`), so it has no
scripts and no performance cost.

Works with vanilla weapons and with any weapon mod: the scanner reads them from the game and mod
files themselves.

## How weapon wear works

Every shot wears the weapon by the ammo's `damageBarrel` divided by the weapon's `barrelArmor`.
A suppressor (or compensator, or other muzzle attachment) has its own `barrelArmor` and wears the
same way. So **10× the barrelArmor = 10× as many shots** before it degrades by the same amount.

This mod only changes `barrelArmor`, never the ammo, so every weapon can be tuned on its own.

## Default settings

- **Weapons last 10× as long.**
- **Suppressors and muzzle attachments last 10× as long**, except the plastic bottle (improvised)
  suppressor, which lasts 3.3× as long: it's meant to be a stopgap, not a permanent fit.
- Every weapon and attachment can be set on its own in `durability.csv`.

## The two settings files

**`settings.json`**: the global multipliers.

| Setting | Default | Meaning |
|---|---|---|
| `WeaponMultiplier` | `10.0` | Every weapon lasts this many times longer |
| `SuppressorMultiplier` | `10.0` | Every suppressor, compensator and muzzle attachment lasts this many times longer |

Use `1.0` for vanilla, or less than 1 to make things wear faster.

**`durability.csv`**: one row per weapon family or attachment. Fill in `Multiplier` to give that
row its own value; leave it blank to use the global one.

| Column | Meaning |
|---|---|
| Type | Weapon, Suppressor or Muzzle (compensators, Groza barrels…) |
| Class | The class that sets its durability |
| Example | The in-game name of one item it covers |
| Items | How many items use this value (all colour and attachment versions) |
| Source | Vanilla, or the mod it comes from |
| CurrentArmor | Its `barrelArmor` now |
| **Multiplier** | **Your multiplier for this row. Blank = use the one in settings.json.** |

`Parent`, `Section` and `RequiredAddon` are used to write the config; don't change them.

Most weapons are one row per weapon family, e.g. `AKM_Base` covers the KA-M in every version. A
few rows are base classes (`Rifle_Base`, `Pistol_Base`, `Weapon_Base`…); they only cover the
items that don't set their own value, like the GP25 grenade launcher.

## Building it

You need **Python 3** and, to sign the mod for your server, **DayZ Tools** (free on Steam).

1. **Change `settings.json` and/or `durability.csv`.**
2. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 WeaponDurability -Key C:\path\to\YourServer.biprivatekey
   ```
   The mod is in `build\@DevZ_WeaponDurability`.
3. **Install it:** copy `@DevZ_WeaponDurability` to your server, its `keys\*.bikey` into the
   server's `keys\` folder, and add it to `-mod=`. Or add `-Install C:\path\to\DayZServer` to
   step 2.

Players need the same copy of the mod, so publish it (Workshop) or give them the folder.

### Prefer editing the config directly?

`src\config.cpp` has one readable line per row with the old value and multiplier in a comment.
Edit the numbers and build with `-NoGenerate` so your edits aren't overwritten:

```cpp
class AKM_Base: Rifle_Base { barrelArmor = 44.44; };  // KA-M, was 4.444 (x10)
```

## Using other weapon mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python WeaponDurability\scan_durability.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeWeaponMod"
```

It rebuilds `durability.csv` from what it finds and **keeps any `Multiplier` you've already set**.

**The table in this repo lists vanilla weapons only**, so the mod runs on any server. Rows you
add by scanning a mod make the build require that mod, so only scan mods your server runs.

## Changing it on a live server

Safe at any time. Weapons keep their current condition; only how fast they wear from now on
changes.
