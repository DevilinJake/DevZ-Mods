# Using DevZ mods on your server

Every DevZ mod is on the Steam Workshop, built from the settings in this repo. You can use it
as it is, or change the settings and make your own copy. Both are fine (see
[LICENSE](LICENSE)).

## Using a mod as it is

1. **Subscribe** to it on the Workshop and copy it into your server folder (or link it the way
   you do your other Workshop mods).
2. **Add it to your start line:**
   - Client mods: add it to `-mod=` and copy its key, `keys\DevZ.bikey` from the mod folder,
     into your server's `keys\` folder. Players download it automatically when they join.
   - Server-side mods (**AutoStack**, **InfectedDamage**): add it to `-servermod=`. Players
     download nothing and no key is needed. AutoStack's settings are in
     `<profiles>\DevZ_AutoStack\Config.json`, so it never needs rebuilding.
3. **AmmoStacks only:** run `fix_loot.py` once so the ammo found in loot stays the same size
   (see [AmmoStacks](AmmoStacks#why-loot-needs-fixing)). You need Python and this repo for that
   step: steps 1–2 below.

The Workshop builds change **vanilla items only**, so they work on any server and don't need
any other mod. RemoveRecipes removes nothing until you choose recipes (it's only useful
customised).

## Changing the settings

The settings are built into the mod's files, and your players need exactly the same files as
the server. So changing a setting means building your own copy of the mod and giving that
copy to your players. It takes about 15 minutes the first time.

### What you need (all free)

- **Python 3** from [python.org](https://www.python.org/downloads/). In the installer, tick
  **"Add python.exe to PATH"**.
- **DayZ Tools** from Steam (Library → Tools). It signs the mod and uploads it to the Workshop.
- **This repo:** on the GitHub page click **Code → Download ZIP** and unzip it anywhere, for
  example `C:\DevZ-Mods`. You don't need Git.

### 1. Open PowerShell in the repo folder

Open the unzipped folder in Explorer, click the address bar, type `powershell` and press Enter.
Every command below runs in that window. If Windows refuses to run `build.ps1`, run this once:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

### 2. Make your own signing key (once)

```powershell
& "C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\DsUtils\DSCreateKey.exe" MyServer
```

This makes `MyServer.biprivatekey` and `MyServer.bikey` in the current folder. **Keep the
`.biprivatekey` to yourself**; the `.bikey` is the half you share. Move both somewhere safe,
for example `C:\Keys`.

### 3. Scan your server (only if you run other mods)

The tables list vanilla items. To add items from your own mods (a clothing mod's backpacks, a
weapon mod's ammo, a crafting mod's recipes), run the mod's scanner with your server folder
first and then each mod folder you want included:

```powershell
python InventorySizes\scan_sizes.py "C:\DayZServer" "C:\DayZServer\@SomeClothingMod"
```

Workshop mods are in `C:\Program Files (x86)\Steam\steamapps\workshop\content\221100\<ID>`
if your server folder only has links to them. The scanner keeps anything you've already filled
in. Each mod's scanner:

| Mod | Scanner | Table it fills |
|---|---|---|
| [InventorySizes](InventorySizes) | `scan_sizes.py` | `sizes.csv` |
| [AmmoStacks](AmmoStacks) | `scan_ammo.py` | `ammo.csv` |
| [WeaponDurability](WeaponDurability) | `scan_durability.py` | `durability.csv` |
| [CombineNoSplit](CombineNoSplit) | `scan_items.py` | `items.csv` |
| [RemoveRecipes](RemoveRecipes) | `scan_recipes.py` | `recipes.csv` |
| [RepairToPristine](RepairToPristine) | `scan_kits.py` | `kits.csv` |
| [InfectedDamage](InfectedDamage) | `scan_attacks.py` | `attacks.csv` |

**Only scan mods your server runs.** A build with rows from a mod requires that mod to be
loaded first.

### 4. Change the settings

Open the table (`.csv`) in Excel, LibreOffice or Google Sheets, fill in your values and save it
as CSV. Some mods also have a `settings.json` with an overall multiplier. Each mod's README says
what every column means.

### 5. Build and install it

```powershell
.\build.ps1 InventorySizes -Key C:\Keys\MyServer.biprivatekey -Install C:\DayZServer
```

This writes the config, packs and signs the mod and copies it into your server as
`@DevZ_InventorySizes`, with `MyServer.bikey` in its `keys\` folder and your server's `keys\`.

**InfectedDamage** is server-side, so it needs no key: `.\build.ps1 InfectedDamage -Install
C:\DayZServer`, load it with `-servermod=` instead of the Workshop copy, and you're done
(skip step 6, and step 2 if it's the only mod you build).

### 6. Give your copy to your players

Players must have your copy, not the Workshop one, so load only one of them. Either:

- **Upload it to the Workshop** with DayZ Tools → **Publisher**: choose the
  `build\@DevZ_<Name>` folder, give it your own title (for example "MyServer Inventory
  Sizes"), and set it to Unlisted if it's only for your server. Then use that Workshop item on
  your server instead of the DevZ one.
- **Or put it in your server pack** if you already make one.

A credit (a link back to this repo) is appreciated but not required.

### Changing it again later

Edit the table, run step 5 again, then update your Workshop item in Publisher (pick it from
the list so it keeps its ID). Restart the server; players download the update when they join.
