# DevZ House Raid Rules

Rebuilt house walls, doors, windows and barricades follow the same PvE and raid-hour rules as
base-building items. An add-on for **Ninjins PvP/PvE Upgraded**: without it the mod has nothing to
read and won't load.

- **Server-side only.** Players don't download anything, and it needs no signing.
- **Fills a gap in Ninjins Raid Extension.** That mod stops bullets and explosives damaging fences,
  gates, watchtowers and tents outside the zones you allow, but only on *items*. Houses rebuilt with
  bricks, mortar and a trowel (Nasdara and any other map with vanilla rebuilding) aren't items: the
  rebuilt wall is a construction part of the house itself. So a rebuilt door could be shot or blown
  open in a PvE zone while the fence next to it couldn't. This mod puts the same check on the house.
- **A stopgap**, written to be easy to adopt or replace: all the zone, admin and settings reading
  goes through Ninjins' public classes, nothing of Ninjins' is copied or changed.

## What it decides

A bullet, explosive or melee hit on a **built** rebuilt part goes through these checks, in order.
Hits on anything else about the house (and parts that aren't built) are left to vanilla.

| Check | Result |
|---|---|
| Rules off (`Enabled` 0, or Ninjins Raid Extension off when following it) | Hit lands |
| That kind of hit isn't blocked (`BlockBulletDamage` and friends) | Hit lands |
| Attacker is a Ninjins admin and admins bypass | Hit lands |
| Zone type at the hit isn't in `DamageAllowedInZoneTypes` (default: PvP and Raid) | **Blocked** ("...only in the PvP zone") |
| `RaidHoursOnly` and it's not raid time | **Blocked** ("...only during raid hours") |
| Otherwise | Hit lands, vanilla takes it from there |

The zone type comes from Ninjins' zone engine at the point the hit landed: a safe zone wins over
everything, then a raid zone, then PvP against PvE by zone priority, then visual zones. Only zones
whose own schedule is active count. Outside every zone the map-wide PvE/PvP schedule in Ninjins'
`GlobalSettings.json` decides, and with neither on the type is `None`.

Raid time is **Expansion's** `RaidSettings.json` schedule when Expansion Base Building is loaded
(so the house parts open and close with Expansion's walls), otherwise the mod's own `Schedule`
(default Friday 18:00 to Sunday 21:00, server time). Running Ninjins raid zones with their own
schedule instead? Set `RaidHoursOnly` to `0` and list only `Raid` in `DamageAllowedInZoneTypes`.

A blocked hit is written to the server log and the attacker gets a short notice. Melee hits aren't
something Ninjins Raid Extension covers; here they're blocked by default so a PvE house can't be
chipped down with an axe. The sledgehammer and dismantling are **actions**, not hits: block
`ActionDestroyPart` and `ActionDismantlePart` in Ninjins Core's `ActionsManager.json` for your
PvE and safe zones, as you would for any base.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 HouseRaidRules
   ```
2. **Copy `build\@DevZ_HouseRaidRules`** into your server folder.
3. **Load it as a server mod** after Ninjins: add it to `-servermod=` in your start line, e.g.
   `-servermod=@DevZ_HouseRaidRules;`

Needs Ninjins PvP/PvE Upgraded in `-mod=` (its Core, Zones and Raid Extension PBOs). Expansion is
optional.

## Settings

Written to `<profiles folder>\DevZ_HouseRaidRules\Config.json` on first start (new settings are
added to it after an update). Edit it with the server stopped.

| Setting | Default | Meaning |
|---|---|---|
| `Enabled` | `1` | `0` turns the whole mod off |
| `FollowNinjinsRaidExtension` | `1` | `1` = the on/off switches below come from Ninjins Raid Extension's `MainConfig.json` (its `Enabled`, `BlockBulletDamage`, `BlockExplosiveDamage`, `BypassForAdmins`), one place to edit. `0` = use the values here |
| `BlockBulletDamage` | `1` | Bullets are subject to the rules (when not following Ninjins) |
| `BlockExplosiveDamage` | `1` | Grenades, charges, traps, 40 mm, Expansion C4/RPG/LAW (when not following Ninjins) |
| `BypassForAdmins` | `0` | `1` = Ninjins admins damage rebuilt parts anywhere, any time (when not following Ninjins) |
| `BlockMeleeDamage` | `1` | Axes, hammers and the like are subject to the rules too (always this mod's own) |
| `DamageAllowedInZoneTypes` | `["PvP", "Raid"]` | Ninjins zone types where hits can land at all: `PvP`, `Raid`, `PvE`, `Safe`, `Visual`, `None`. Everywhere else they're blocked |
| `RaidHoursOnly` | `1` | Inside those zones, hits only land during raid hours |
| `UseExpansionRaidSchedule` | `1` | Raid hours are Expansion's `RaidSettings.json` schedule when Expansion Base Building is loaded |
| `Schedule` | Fri 18:00 +360 min, Sat 0:00 +1440, Sun 0:00 +1260 | The mod's own raid windows, used when Expansion's aren't: `Weekday` (`""` = every day), `StartHour`, `StartMinute`, `DurationMinutes`, server local time. Empty = always raid hours |
| `ShowBlockedNotify` | `1` | Tell the attacker why the hit did nothing |
| `NoticeWaitSeconds` | `3` | At most one notice per player this often |
| `BlockedZoneText` | `Rebuilt house parts can only be raided in the PvP zone` | |
| `BlockedHoursText` | `Rebuilt house parts can only be raided during raid hours` | |
| `LogBlocked` | `1` | Write each blocked hit to the server log (`[DevZ_HouseRaidRules] DENIED ...` lines with house, part, zone, kind, ammo, attacker and reason) |

## How it works, for modders

Vanilla `BuildingBase` creates a `Rebuilding` construction component; each rebuildable part is a
`ConstructionPart` whose name is also the house's damage zone for it. When a zone is ruined,
`ConstructionBase.OnHealthLevelChanged` destroys the part. This mod overrides
`BuildingBase.EEOnDamageCalculated` (server only), looks the zone up as a part, and if the part
is built asks `DevZ_HouseRaidRules.MayDamagePart`; returning `false` cancels the damage before
any health is taken. Zones come from `NinjinsUpgraded_ZoneEngineModule.GetEngine()`, the
attacker from `NinjinsUpgraded_HitSource`, the admin flag from `PlayerBase.NinjinsUpgraded_IsAdmin()`
and the switches from `g_NinjinsRaidExt_Config`. Two files: `DevZ_HouseRaidRulesConfig.c`
(settings and the schedule) and `DevZ_HouseRaidRules.c` (the rules), plus the one-method
`BuildingBase.c` hook.
