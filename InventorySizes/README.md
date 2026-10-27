# DevZ Inventory Sizes

Sets your own cargo sizes for clothing, backpacks, vest pouches and holsters, like Inventory++,
but with the numbers you choose. It only overrides one value per item (`itemsCargoSize`), so it has no scripts,
no performance cost and little to break when DayZ updates.

Works with vanilla items and with any clothing mod: the scanner reads the items from the game
and mod files themselves.

## Default sizes

The sizes in `sizes.csv` are the ones I use on my Nasdara server:

- **Most items get a little less than double their vanilla space**, typically 50–90% more.
- **The largest backpacks go further** (Field, Long Haul, Tactical and Mountain backpacks, up to
  10×16), so they can carry large items like sheet metal, barrels and vehicle parts.
- **Every size is configurable.** Change any row in `sizes.csv`, or clear the new size to keep
  vanilla.

## How it works

Every item's cargo grid is set on a "base" class, and all its colour versions inherit it.
For example, `AssaultBag_ColorBase` sets the grid for the Assault Backpack in every colour.
`sizes.csv` has one row per grid, so you change one row and every version changes with it.

| Column | Meaning |
|---|---|
| Category | Backpacks, Tops, Vests, Pants, Belts, Pouches, Holsters… |
| Class | The class that sets the grid |
| Example | The in-game name of one item that uses it |
| Items | How many items use this grid |
| Source | Vanilla, or the mod it comes from |
| CurrentWidth / CurrentHeight / CurrentSlots | The size now |
| **NewWidth / NewHeight** | **Your size. Leave blank to keep the current one.** |

`Parent` and `RequiredAddon` are used to write the config; don't change them.

The inventory panel is 10 columns wide, so keep widths at 10 or less.

## Changing sizes

You need **Python 3** and, to sign the mod for your server, **DayZ Tools** (free on Steam).

1. **Open `sizes.csv`** in Excel, LibreOffice or Google Sheets.
2. **Fill in `NewWidth` and `NewHeight`** for the items you want to change. Save as CSV.
3. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 InventorySizes -Key C:\path\to\YourServer.biprivatekey
   ```
   This writes `src\config.cpp` from the table, packs it and signs it. The finished mod is in
   `build\@DevZ_InventorySizes`.
4. **Install it:** copy `@DevZ_InventorySizes` into your server folder, its `keys\*.bikey`
   into the server's `keys\` folder, and add it to `-mod=`. Or add
   `-Install C:\path\to\DayZServer` to step 3 to do this for you.

Players need the same copy, so publish it (Workshop) or give them the folder.

### Prefer editing the config directly?

`src\config.cpp` is written to be read by people: one line per item, with the old size in a
comment. You can edit the numbers there and build with `-NoGenerate` so the table doesn't
overwrite your edits:

```cpp
class AssaultBag_ColorBase: Clothing { itemsCargoSize[] = {8, 8}; };  // Assault Backpack, was 6x6
```

## Adding items from other mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python InventorySizes\scan_sizes.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeClothingMod"
```

It adds the new items to `sizes.csv` and **keeps the sizes you've already filled in**. When you
build, the new mod is added to the config's `requiredAddons` for you, so it loads in the right order.

Vests, belts and holsters with no grid of their own (e.g. the plate carrier, whose storage
comes from its pouches) are listed with blank Current columns. Fill in a size to give them
pockets. Any other item can be added the same way: a row with its `Class`, `Parent` and
`RequiredAddon`, blank Current columns and your new size.

## Before you change sizes on a live server

**Only make grids bigger once players have stored things.** If a grid shrinks, items that no
longer fit can be pushed out of it. Set your sizes before launch, or only increase them.

## Your own signing key

Servers with `verifySignatures=2` only load mods signed with a key they trust. Make one with
DayZ Tools: `Bin\DsUtils\DSCreateKey.exe YourServer`. Keep the `.biprivatekey` private and
share only the `.bikey`.
