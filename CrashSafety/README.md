# DevZ Crash Safety

Vehicle crashes hurt but don't end everything. The crew are hurt and knocked out instead of
killed, and the vehicle ends up badly damaged instead of ruined, so it can be repaired.

- **Server-side only.** Players don't download anything, and it needs no signing.
- **Works with every vehicle**: vanilla cars, trucks, motorbikes and boats, and modded ones built
  on them, including Expansion's helicopters and boats.
- **Parts still break.** Only the parts that would end a vehicle for good are protected: the
  hull, engine and fuel tank. Rotors, wheels, doors, windows and the rest still take full damage
  and can be ruined, so a bad crash still costs you.

## The crew

Vanilla kills everyone inside above a set impact. With this mod a heavy crash:

| | Light heavy crash | Worst crash |
|---|---|---|
| Health lost | 10% of max | 70% of max |
| Knocked out | No (below severity 0.3) | 45 s |

Severity runs from 0 (the smallest heavy crash) to 1 (a crash that would kill in vanilla). A
crash never takes a player below 1 health. Players who are already badly hurt can still die of
bleeding afterwards.

## The hull, engine and fuel tank

A vehicle has an overall health (the hull) and separate parts. Three of them end a vehicle for
good once they're **Ruined**:

| Part | Ruined in vanilla |
|---|---|
| Hull | The vehicle is destroyed. Expansion helicopters catch fire |
| Engine | Can't be repaired or replaced: the vehicle never drives again |
| Fuel tank | Can't be repaired or replaced, and the game then ruins the engine too |

A helicopter that rolls over with its rotor still turning keeps taking damage until the rotor
stops, so in vanilla even a slow tip-over on landing destroys it.

With this mod these three parts stop at **10%** (Badly Damaged) instead, whatever the cause:
crashes, falls, fire, over-revving, running without coolant or driving into deep water. Boat
decay is left alone: a boat nobody goes near still rots to Ruined, so the loot economy can
respawn it elsewhere. Badly Damaged can be repaired: a blowtorch for a car's engine; epoxy putty,
planks or a blowtorch for the body and fuel tank; a pipe wrench for Expansion helicopters and
boats. A badly damaged fuel tank still leaks while the engine runs. Every other part takes its
damage as normal.

Weapons and explosives aren't held back by default, so vehicles can still be destroyed in a
fight. Use your PvP/PvE mod to decide where vehicles can be shot at all, or set
`FloorsAgainstWeapons` to `1` so nothing ever ruins these parts.

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

**Vehicles that report every crash on one zone:** some models (Expansion's Land Rover) report
all collisions on a single catch-all zone like `Chassis`, which has no wheels or body parts linked
to it, so crashes damage nothing you can see and never hurt the crew. Crashes on the zones listed
in `CatchAllZones` are moved to the body zone nearest the impact (front, back, a fender...), and
the game's own damage rules take it from there.

**Expansion Vehicles:** Expansion runs its own engine simulation, where low coolant (and low oil
in a helicopter) wears the engine down every tick. That's held at the floor too, so a helicopter
flown without oil ends up with a Badly Damaged engine instead of being destroyed. Keep Expansion's
`RevvingOverMaxRPMRuinsEngineInstantly` off (the default): it skips the floors. And set
`CollisionDamageMinSpeedKmh` to `0`: Expansion's minimum-speed check zeroes every crash in which
the vehicle slows down (it compares a negative momentum change), so cars take no crash damage at
all and nothing here ever runs. Vanilla's own minimum still ignores small bumps.

## Settings

Written to `<profiles folder>\DevZ_CrashSafety\Config.json` the first time a vehicle takes
damage (new settings are added to it after an update). Edit it with the server stopped.

| Setting | Default | Meaning |
|---|---|---|
| `Enabled` | `1` | `0` turns the whole mod off (vanilla crashes) |
| `MinHealthLossPercent` | `10` | Health lost in the lightest heavy crash, % of max |
| `MaxHealthLossPercent` | `70` | Health lost in the worst crash, % of max |
| `KnockoutFromSeverity` | `0.3` | Crashes at or above this severity (0 to 1) knock the crew out |
| `MinKnockoutSeconds` | `15` | Knockout time at that severity |
| `MaxKnockoutSeconds` | `45` | Knockout time in the worst crash |
| `CarSeverityStart` | `750` | Crash damage that counts as severity 0 for cars (vanilla heavy crash) |
| `CarSeverityFull` | `1200` | Crash damage that counts as severity 1 for cars (vanilla kills here) |
| `BikeSeverityStart` | `40` | The same for motorbikes |
| `BikeSeverityFull` | `80` | |
| `HullFloorPercent` | `10` | The hull never drops below this % except to weapons (Badly Damaged is under 30). `0` turns it off |
| `EngineFloorPercent` | `10` | The same for the engine |
| `FuelTankFloorPercent` | `10` | The same for the fuel tank |
| `FloorsAgainstWeapons` | `0` | `1` = guns and explosives can't ruin these parts either |
| `CatchAllZones` | `["Chassis"]` | Zones some vehicle models report every crash on, passing nothing to the body or crew (Expansion's Land Rover). A crash on one of these counts against the body zone nearest the impact instead |
| `LogCrashes` | `1` | Write each crash and each held part to the server log (`[DevZ_CrashSafety]` lines) |
