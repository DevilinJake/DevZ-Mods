# DevZ Essentials

Eight DevZ client mods in one download, for servers that want all of them and fewer entries in
the launcher:

| Mod | What it does |
|---|---|
| [Inventory Sizes](../InventorySizes) | More room in clothing, backpacks, vests and pouches |
| [Ammo Stacks](../AmmoStacks) | Loose ammo stacks twice as big |
| [Weapon & Suppressor Durability](../WeaponDurability) | Weapons and suppressors last 10× as long (the plastic bottle suppressor 3.3×) |
| [Combine Kits](../CombineNoSplit) | Combine part-used bandages, repair kits and filters, but never split them |
| [Repair To Pristine](../RepairToPristine) | Repair kits repair items all the way to Pristine |
| [Quick Transfer](../QuickTransfer) | Hold Left Alt and sweep the mouse over items to move them between you and the nearest open container |
| [Auto Run](../AutoRun) | Press Z and your character keeps moving forward at the speed you had |
| [Battery Life](../BatteryLife) | Batteries hold twice as much, so every device on them lasts twice as long |

It's the same files as the single mods, with the same default settings: each mod's README
explains them. **Load either the bundle or the single mods, never both.**

## Using it

1. Add `@DevZ_Essentials` to `-mod=` and copy its `keys\DevZ.bikey` into your server's `keys\`.
2. Ammo Stacks: run `fix_loot.py` once so loot stays the same size (see
   [AmmoStacks](../AmmoStacks#why-loot-needs-fixing)).
3. Running DayZ Expansion? It has its own auto-run on Z: set `EnableAutoRun` to `0` in
   Expansion's `GeneralSettings.json` (see [AutoRun](../AutoRun#the-key)).

## Your own settings

Change the tables of the mods inside it (see [CUSTOMISING.md](../CUSTOMISING.md)), then build
the bundle, which builds each mod and puts them together:

```powershell
.\build_pack.ps1 Essentials -Key C:\Keys\MyServer.biprivatekey -Install C:\DayZServer
```

To leave a mod out or add one, edit `pack.txt` (one mod folder per line).
