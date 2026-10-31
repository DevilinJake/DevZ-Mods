# DevZ Infected Damage

Makes zombies hit harder (or softer). Set one multiplier each for Health, Blood and Shock
damage, bleed chance and clothing wear, and change any single attack you want to treat
differently. It only overrides config values, so it has no scripts and no performance cost.

Works with vanilla zombies and with zombie mods: the scanner finds every attack the infected
actually use, straight from the game and mod files.

## How a zombie hit works

Every hit takes something off three bars at once:

| Bar | A player has | What it means |
|---|---|---|
| **Health** | 100 | At 0 you die |
| **Blood** | 5000 | Low blood makes you dizzy, then unconscious, then dead. Most blood is lost through bleeding cuts, not the hit itself |
| **Shock** | 100 | When it runs out you're knocked out. It refills quickly, so only fast repeated hits matter |

Each hit also has a **chance to start bleeding** and **wears the clothing** it lands on.
Clothing with armour soaks up part of every hit, so dressed players take less and bleed less.

Each zombie type has two attacks: a swipe (standing, running and crawling) and a lunge from a
short distance away. Vanilla values:

| Attack | Used by | Health | Shock | Bleed chance* |
|---|---|---|---|---|
| `MeleeInfected` / `MeleeInfectedLong` | Most zombies (swipe / lunge) | 6.5 / 8.5 | 12 | 11% |
| `MeleeSoldierInfected` / `...Long` | Soldier zombies | 8.5 / 11 | 12 / 16 | 17.5% |
| `MeleePoliceInfected` / `...Long` | Police zombies | 8 / 8 | 12 / 15 | 15% |
| `MeleeMummyInfected` / `...Long` | The mummy (event zombie) | 10 / 10 | 35 | 100% |

\*Per hit, on a player with no clothing on. Every attack does 100 Blood.

## Default settings

Health and Shock a rough 50% harder:

- **Health 1.5×**: 11 swipes or 8 lunges kill a player with no armour (vanilla 16 / 12).
- **Shock 1.5×**: 18 per hit, so a group can knock you out with quick hits.
- **Bleed chance 1×**: unchanged (11% per hit). Zombie virus and wound infection mods (such as
  ZenVirus) infect on bleeding hits, so a higher bleed chance makes them much harsher.
- **Blood 1×, clothing wear 1×**: unchanged.

Soldier and police zombies keep hitting harder than the rest, by the same proportion.

## The two settings files

**`settings.json`**: the global multipliers.

| Setting | Default | Meaning |
|---|---|---|
| `HealthMultiplier` | `1.5` | Health damage per hit |
| `ShockMultiplier` | `1.5` | Shock damage per hit (knock-outs) |
| `BloodMultiplier` | `1.0` | Blood taken by the hit itself (not by bleeding) |
| `BleedChanceMultiplier` | `1.0` | Chance per hit to start bleeding |
| `ClothingDamageMultiplier` | `1.0` | How much the hit wears the clothing it lands on |

Use `1.0` for vanilla, less than 1 to make zombies weaker. Bleed chance can't go above 100%.

**`attacks.csv`**: one row per attack. Fill in `Health`, `Blood`, `Shock`, `Bleed` or `Clothing`
to give that attack its own multiplier; leave it blank to use the global one.

| Column | Meaning |
|---|---|
| Class | The attack |
| Attacks | Which zombie moves use it (`AttackLong` is the lunge) |
| Zombies | How many zombie types use it |
| Source | Vanilla, or the mod it comes from |
| Current... | Its values now (`CurrentClothing` is `armorDamage`) |
| **Health, Blood, Shock, Bleed, Clothing** | **Your multipliers for this attack. Blank = use settings.json.** |

`Parent`, `RequiredAddon` and `DamageType` are used to write the config; don't change them.

### How the bleed chance is worked out

The game decides bleeding from the attack's `bleedThreshold` and its Blood damage, through a
fixed chance table (`BleedChanceData.c`). The generator works out the `bleedThreshold` that gives
your chance for a player with no clothing on, and writes the result as a comment next to it.

## Building it

You need **Python 3**. It's server-side only: players don't download it and it doesn't need
signing.

1. **Change `settings.json` and/or `attacks.csv`.**
2. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 InfectedDamage
   ```
   The mod is in `build\@DevZ_InfectedDamage`.
3. **Install it:** copy `@DevZ_InfectedDamage` to your server and add it to `-servermod=`. Or add
   `-Install C:\path\to\DayZServer` to step 2.

### Prefer editing the config directly?

`src\config.cpp` has one block per attack with the old values in a comment. Edit the numbers and
build with `-NoGenerate` so your edits aren't overwritten.

## Using zombie mods

Run the scanner with your game or server folder first, then each mod folder:

```powershell
python InfectedDamage\scan_attacks.py "C:\...\DayZServer" "C:\...\DayZServer\@SomeZombieMod"
```

It rebuilds `attacks.csv` from what it finds and **keeps any multipliers you've already set**.
The table in this repo is vanilla only, so it works on any server.

## Changing it on a live server

Safe at any time (after a restart). Only hits from then on change.
