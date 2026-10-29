# DevZ Crash Safety

Vehicle crashes hurt but don't end everything. The crew are hurt and knocked out instead of
killed, and the vehicle ends up badly damaged instead of ruined, so it can be repaired.

- **Server-side only.** Players don't download anything, and it needs no signing.
- **Works with every vehicle**: vanilla cars, trucks, motorbikes and boats, and modded ones built
  on them, including Expansion's helicopters and boats.
- **Parts still break.** Only the hull is protected. Engines, fuel tanks, rotors, wheels and
  doors still take full damage and can be ruined, so a bad crash still costs you.

## The crew

Vanilla kills everyone inside above a set impact. With this mod a heavy crash:

| | Light heavy crash | Worst crash |
|---|---|---|
| Health lost | 10% of max | 70% of max |
| Knocked out | No (below severity 0.3) | 60 s |

Severity runs from 0 (the smallest heavy crash) to 1 (a crash that would kill in vanilla). A
crash never takes a player below 1 health. Players who are already badly hurt can still die of
bleeding afterwards.

## The hull

A vehicle has an overall health (the hull) and separate parts. In vanilla, a hard enough crash
takes the hull to 0: the vehicle is **Ruined**, can't be repaired, and Expansion helicopters
catch fire. A helicopter that rolls over with its rotor still turning keeps taking damage until
the rotor stops, so even a slow tip-over on landing ruins it.

With this mod, crashes, falls and fire stop at **10% hull** (Badly Damaged). Everything that hit
still damages the parts as normal.

Weapons and explosives aren't held back by default, so vehicles can still be destroyed in a
fight. Use your PvP/PvE mod to decide where vehicles can be shot at all, or set
`HullFloorAgainstWeapons` to `1` so nothing ever ruins a hull.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 CrashSafety
   ```
2. **Copy `build\@DevZ_CrashSafety`** into your server folder.
3. **Load it as a server mod**, not a normal mod: add it to `-servermod=` in your start line,
   e.g. `-servermod=@DevZ_CrashSafety;`

Leave other mods' crew damage settings (for example Expansion's `VehicleCrewDamageMultiplier`)
at `1`, since this mod replaces crew crash damage.

## Settings

Written to `<profiles folder>\DevZ_CrashSafety\Config.json` the first time a vehicle takes
damage (new settings are added to it after an update). Edit it with the server stopped.

| Setting | Default | Meaning |
|---|---|---|
| `Enabled` | `1` | `0` turns the whole mod off (vanilla crashes) |
| `MinHealthLossPercent` | `10` | Health lost in the lightest heavy crash, % of max |
| `MaxHealthLossPercent` | `70` | Health lost in the worst crash, % of max |
| `KnockoutFromSeverity` | `0.3` | Crashes at or above this severity (0 to 1) knock the crew out |
| `MinKnockoutSeconds` | `30` | Knockout time at that severity |
| `MaxKnockoutSeconds` | `60` | Knockout time in the worst crash |
| `CarSeverityStart` | `750` | Crash damage that counts as severity 0 for cars (vanilla heavy crash) |
| `CarSeverityFull` | `1200` | Crash damage that counts as severity 1 for cars (vanilla kills here) |
| `BikeSeverityStart` | `40` | The same for motorbikes |
| `BikeSeverityFull` | `80` | |
| `HullFloorPercent` | `10` | Crashes, falls and fire never take the hull below this % (Badly Damaged is under 30). `0` turns it off |
| `HullFloorAgainstWeapons` | `0` | `1` = guns and explosives can't ruin the hull either |
| `LogCrashes` | `1` | Write each crash and each held hull to the server log (`[DevZ_CrashSafety]` lines) |
