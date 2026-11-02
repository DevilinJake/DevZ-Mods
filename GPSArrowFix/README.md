# DevZ GPS Arrow Fix

Fixes a bug in **DayZ Expansion** (Navigation): your player arrow on the GPS mini-map disappears
after you open the inventory, the map, a trader or any other menu, and only comes back when you
cycle the GPS with its key. With this mod the arrow comes back as soon as the menu closes.

- **Players need it too** (it's a normal mod, not a server-only one): the GPS is drawn on each
  player's own game, so the fix has to be there.
- **Needs Expansion Navigation.** Without it the mod compiles to nothing and does no harm.
  Load it **after** Expansion.
- **Changes nothing else.** No items, no settings, no Expansion files: it's a small script
  override, the same way every Expansion add-on works.

## The bug

Expansion's GPS HUD (`IngameHud`) checks the arrow every frame. While a menu is open it hides
the arrow, so it doesn't draw over the menu. The check for "no menu open" was meant to bring it
back, but it calls `Hide()` too. So once you open any menu the arrow stays hidden until
something redraws the whole GPS, which is what cycling it does.

## The fix

One small override of `IngameHud.Update()`, in
`src\scripts\5_Mission\DevZ_GPSArrowFix\IngameHud.c`: after Expansion's own code runs, if the
arrow is hidden but Expansion's own rules say it should show (no menu or inventory open, GPS on,
mini-map showing, the server shows player positions), it's shown again.

The whole file is wrapped in `#ifdef EXPANSIONMODNAVIGATION`, Expansion's own marker, so
without Expansion Navigation nothing is compiled. The comment at the top of the file describes
the bug in Expansion's terms, so its team can lift the fix if they want it.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 GPSArrowFix -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_GPSArrowFix`** into your server folder, add it to `-mod=` **after
   Expansion**, and copy its key from `keys\` into your server's `keys\`.
3. Players get the same copy (Workshop or your server pack).

## Settings

None. There's nothing for admins to set. The arrow still follows Expansion's own
`MapSettings.json` (`EnableHUDGPS`, `ShowPlayerPosition`).

## Testing it

Hold a GPS, open the GPS mini-map, then open and close your inventory. The arrow should be back
the moment the inventory closes. Without the mod it stays gone until you cycle the GPS.
