# DevZ Mods

Small, open-source DayZ mods by DevilinJake (Dev). They work on any map; I use them on my
Nasdara server. Each one is in its own folder,
builds on its own and is set up so server admins can change the numbers without touching code.

| Mod | What it does | Status |
|---|---|---|
| [InventorySizes](InventorySizes) | Your own cargo sizes for clothing, backpacks and pouches (defaults: a little under double vanilla, bigger for the largest backpacks) | Ready |
| [AmmoStacks](AmmoStacks) | Bigger loose-ammo stacks: one multiplier, per-ammo overrides, loot amounts kept the same | Ready |
| [AutoStack](AutoStack) | Loose ammo merges into your existing stacks automatically (server-side only) | Ready |
| [WeaponDurability](WeaponDurability) | How fast weapons and suppressors wear: one multiplier each (default 10x), per-weapon overrides | Ready |
| [InfectedDamage](InfectedDamage) | How hard zombies hit: Health, Blood, Shock, bleed chance and clothing wear multipliers (default 1.5x damage), per-attack overrides (server-side only) | Ready |
| [CrashSafety](CrashSafety) | Vehicle crashes hurt and knock out the crew instead of killing them, and leave the vehicle Badly Damaged instead of Ruined (parts still break) (server-side only) | Ready |
| [HouseRaidRules](HouseRaidRules) | Rebuilt house walls, doors and windows (vanilla rebuilding, e.g. Nasdara) follow Ninjins PvP/PvE Upgraded's PvE and raid-hour rules against bullets, explosives and melee; add-on for Ninjins (server-side only) | Ready |
| [CombineNoSplit](CombineNoSplit) | Combine part-used bandages, repair kits and gas mask filters, but never split them; a scanner lists items from any mod to pick from | Ready |
| [RemoveRecipes](RemoveRecipes) | Switch off right-click crafting recipes from vanilla or any mod, without editing it; a scanner lists every recipe to pick from | Ready |
| [RepairToPristine](RepairToPristine) | Repair kits repair items all the way to Pristine instead of stopping at Worn; a scanner lists kits from any mod to pick from | Ready |
| [QuickTransfer](QuickTransfer) | Hold a key (Left Alt) with the inventory open and sweep the mouse over items: each one moves between you and the nearest open container | Ready |
| [AutoRun](AutoRun) | Press a key (Z) to keep moving forward at the speed you have (walk, jog or sprint); any movement key stops it, jumping and crouching don't | Ready |
| [SatchelSoundFix](SatchelSoundFix) | Fixes a DayZ Expansion bug: the satchel charge's arming beep no longer carries on squealing after it explodes (needs Expansion Base Building; does nothing without it) | Ready |
| [HouseCodeLock](HouseCodeLock) | The DayZ Expansion code lock on rebuilt vanilla house doors (set code, lock, unlock with Expansion's keypad), until Expansion supports them. Needs Expansion | Testing |
| [Essentials](Essentials) | Bundle: Inventory Sizes, Ammo Stacks, Weapon & Suppressor Durability, Combine Kits, Repair To Pristine and Quick Transfer in one download (built with `build_pack.ps1`) | Ready |

## Using a mod

Every mod is on the Steam Workshop, built with vanilla items only so it works on any server.
Client mods: add them to `-mod=` and copy `keys\DevZ.bikey` into your server's `keys\`.
Server-side mods (AutoStack, InfectedDamage, CrashSafety, HouseRaidRules): add them to `-servermod=`; players download
nothing and no key is needed.

Want different numbers, or items from your own mods? See **[CUSTOMISING.md](CUSTOMISING.md)**:
download this repo, scan your server, change a table and build your own copy. In short:

1. Change its settings file (for example `InventorySizes\sizes.csv`).
2. Build it: `.\build.ps1 <ModName> -Key C:\path\to\YourServer.biprivatekey`
3. Copy `build\@DevZ_<ModName>` to your server and its key into `keys\`, and give players
   the same copy (your own Workshop upload or server pack).

You need **Python 3** and, for signing, **DayZ Tools** (free on Steam).

## Repacking and changing these mods

You're free to use, change and repack these mods, including on your own server or in your own
mod pack. See [LICENSE](LICENSE). A credit is appreciated but not required.

## Repo layout

```
<ModName>\
    README.md        how to use it
    src\             what goes into the PBO (config.cpp, scripts)
    mod.cpp          name and description shown in the launcher
    *.csv / *.py     settings table and the script that turns it into config.cpp
tools\               shared Python helpers (PBO packing, config reading)
build.ps1            builds any mod: .\build.ps1 <ModName>
```
