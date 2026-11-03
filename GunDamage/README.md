# DevZ Gun Damage

Longer, more readable firefights. Rifles knock players out less easily, sniper rifles hit harder
and keep their damage at long range, SMGs and pistols become worth carrying, and slugs stay
useful out to about 150 m. **Bullet flight is never changed**: speed, drop, zeroing and scope
marks are all vanilla.

**Server-side only.** Players don't download anything.

## How a gunshot works

Every hit takes something off three bars at once:

| Bar | A player has | What it means |
|---|---|---|
| **Health** | 100 | At 0 you die |
| **Blood** | 5000 | Mostly lost through bleeding afterwards |
| **Shock** | 100 | At 25 or less you're knocked out; you wake when it's back to 50. It refills quickly while you're awake |

Armour lets through a share of each: a vanilla plate carrier lets through 30% of Health and 60%
of Shock, which is why one sniper shot knocks you out. A bullet also does less damage the more it
has slowed down over distance.

## What the mod changes

**1. Bullet damage** (`ammo.csv` → the mod's config). Every gun firing that bullet changes:

| Bullet | Guns | Change |
|---|---|---|
| 5.56, 5.45, 7.62×39 | Assault rifles | Health and Shock ×0.9 |
| .308, 7.62×54 | Sniper rifles (and FAL, M14, SCAR-H, Longhorn) | Health ×1.45 |
| 9×19, .45, .380 | SMGs and pistols | Health and Shock ×1.25 |
| .357 | Deagle, Magnum, Repeater | Health and Shock ×1.15 |
| 12 gauge slug | Shotguns | Health 110 → 130 |

**2. Armour** (`armour.csv`): the plate carrier lets through 35% of Shock instead of 60%. Assault
rifles need 3 hits to knock out instead of 2, and a sniper chest shot no longer knocks out.

**3. Per-gun damage and range floors** (settings file, a small script). Damage belongs to the
bullet, so the FAL shares the sniper buff and the MP5 shares the pistol one. The script gives
single guns their own multiplier, and lets chosen guns keep their damage at range:

| Gun | Multiplier | Range floor | Why |
|---|---|---|---|
| FAL, M14, SCAR-H | ×0.7 | | Keeps .308 battle rifles near vanilla (3 hits vs a plate carrier, like ARs) |
| Longhorn | ×1.25 | | The alpha-days powerhouse pistol |
| MP5K, UMP45, CZ61, PP19 | ×1.12 | | SMGs at +40% in total (ammo +25% × 1.12) |
| Bolt-action rifles, SVD, Blaze | | 85% | Long shots keep at least 85% of point-blank damage |
| 12 gauge slug (bullet) | | 70% | Slugs stay useful out to about 150 m |

How it works: just before a gunshot's damage lands on a player, the game lets mods cancel it. If
that gun or bullet has its own multiplier, or the bullet slowed down past its floor, the mod
cancels the hit and applies it again straight away at the new strength. The new hit goes through
the normal damage system, so armour, bleeding, broken legs, the flinch, the pain sound and kill
credit all happen once, as normal. Blood splashes and impact sounds come from the bullet itself on
players' PCs and aren't touched. Hits the mod doesn't change (most of them) aren't touched at all.

### Shots to knock out or kill (plate carrier, chest, close range)

| Gun | Vanilla | With this mod |
|---|---|---|
| Assault rifle (5.56) | knocked out by the 2nd hit | knocked out by the 3rd |
| Sniper (.308, 7.62×54) | knocked out by the 1st shot | not knocked out; 2nd shot kills |
| FAL / M14 | knocked out by the 1st hit | knocked out by the 3rd, killed by the 3rd |
| Longhorn | knocked out by the 2nd, killed by the 4th | killed by the 2nd |
| SMG vs press vest | knocked out by the 3rd, killed by the 5th | knocked out by the 2nd, killed by the 4th |

These are worked out from the game's numbers; distance and where the bullet lands move them by
a hit either way.

## Settings

`profiles\DevZ_GunDamage\Config.json`, created on first start. Edit it and restart.

| Setting | What it does |
|---|---|
| `Enabled` | 0 = no per-gun damage or range floors (the ammo and armour changes stay; remove the mod to undo those) |
| `Weapons` | One entry per gun: `ClassName` (matches that gun and every gun built on it, e.g. `BoltActionRifle_Base`), `DamageMultiplier`, `RangeFloor` (0 to 1; 0 = normal drop-off). The first match wins |
| `Ammo` | The same, matched on the exact bullet name. Combined with the gun's entry: multipliers multiply, the higher floor wins |
| `LogHits` | 1 = a line in the server's script log for every hit the mod changes, with the damage before and after |
| `TestReapplyAll` | Testing only: also re-applies hits the mod wouldn't change, at their normal strength, so the log shows a re-applied hit matches the original |

To change bullet or armour damage, edit `ammo.csv` / `armour.csv`, run `python generate_config.py`
(or build with `build.ps1`, which runs it). `scan_ammo.py` re-reads the game (and any weapon mods
you point it at) and refreshes `ammo.csv`, keeping your multipliers.

Knockout time is a mission setting, not part of this mod: `shockRefillSpeedUnconscious` in
`cfggameplay.json` (vanilla 1.0; 0.66 = knocked out about 1.5× as long).

## Install

```powershell
.\build.ps1 GunDamage -Install C:\path\to\DayZServer
```

Add `@DevZ_GunDamage` to the server's `-servermod=` list.

## Checking it works

On start the script log shows the values the game loaded, e.g.
`[DevZ_GunDamage] loaded: PlateCarrierVest bullet armour Health 0.3 Shock 0.35, ...`. With
`LogHits` 1, each changed hit logs the damage the original hit would have done and what the
re-applied hit did, with the expected and actual ratio. The server's admin log (`.ADM`) has one
"hit by" line per hit that landed: one shot should give one line.
