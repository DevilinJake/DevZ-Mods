# DevZ Remove Recipes

Switch off right-click crafting recipes (the ones you get by dragging one item onto another, or
holding one and using the other) from vanilla or from any mod, **without editing that mod**.

Handy when a mod has a shortcut recipe that skips its own crafting stations or rare
ingredients. For example, iM7s Drug Running lets you make cocaine by combining Coca Leaves with
Acetone in your hands. That gives *more* cocaine per leaf than its Chemical Barrel and skips the
barrel's Sulfuric Acid. Removing those two recipes means cocaine has to come from the barrel.

- **Players need it too** (it's a normal mod, not a server-only one). The game numbers every
  recipe and the player's game and the server must agree on the list, so both run this mod.
- **Works with recipes from any mod.** A scanner lists every recipe in the game and your mods,
  and you pick which ones to remove.
- **Bench and station recipes are separate.** Mods like Crafting Plus keep their workbench
  recipes in their own settings file; remove those there. This mod is for right-click recipes.

## Default recipes

| Recipe | Class | Mod |
|---|---|---|
| Make Coca Paste (fresh leaves + Acetone) | `iM7sDR_CraftCocaPaste` | iM7s Drug Running |
| Make Coca Paste (dried leaves + Acetone) | `iM7sDR_CraftCocaPaste_Dried` | iM7s Drug Running |

If you don't run that mod, the server log just says those two weren't found and nothing else
changes.

## Changing the list

The list is `recipes.csv`. Open it in Excel or any text editor. Put `Yes` in the **Remove**
column for each recipe you want gone, and leave it empty for the rest.

| Column | Meaning |
|---|---|
| `Remove` | `Yes` = take this recipe out. Empty = keep it |
| `Class` | The recipe's class name (what the mod's script calls it) |
| `Name` | What players see in the action menu |
| `Item1`, `Item2` | The two things you combine (a few examples when many items work) |
| `Results` | What it makes |
| `Source` | `Vanilla`, or the mod it comes from |

Then build the mod (below). Recipes set to `Yes` show at the top of the file.

### Prefer editing the config directly?

The build writes the list into `src\config.cpp`, one class name per line:

```cpp
class CfgDevZ_RemoveRecipes
{
	recipes[] =
	{
		"iM7sDR_CraftCocaPaste",
		"iM7sDR_CraftCocaPaste_Dried"
	};
};
```

You can edit that by hand and build with `-NoGenerate` so it isn't overwritten.

## Adding recipes from other mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python RemoveRecipes\scan_recipes.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeMod"
```

It adds every recipe from those mods to `recipes.csv` and **keeps the choices you've already
made**. Then set the new ones you want to `Yes` and build.

Mods don't need to load before this one: the recipes are removed after every mod has added its
own. When the server starts, its log (`*.RPT`) shows a `[DevZ_RemoveRecipes] Removed recipe ...`
line for each one.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 RemoveRecipes -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_RemoveRecipes`** into your server folder and its key into `keys\`.
3. **Add it to `-mod=`** in your start line, e.g. `-mod=@CF;...;@DevZ_RemoveRecipes;`
4. **Players add it in their launcher** like any other mod.

Changing the list later means rebuilding the mod, so players download the new version.

## Your own signing key

Servers with `verifySignatures=2` only load mods signed with a key they trust. Make one with
DayZ Tools: `Bin\DsUtils\DSCreateKey.exe YourServer`. Keep the `.biprivatekey` private and
share only the `.bikey`.
