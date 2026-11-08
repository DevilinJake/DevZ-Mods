# DevZ RA Patch

Gives [RA Base Building](https://steamcommunity.com/sharedfiles/filedetails/?id=3173739565) an admin settings file, and adds to it: desk modules from any item, an HQ dashboard, a Discord raid alarm per team, sleeping bags and respawning, a workbench with its own menu, code locks for any car, door and gate limits, material drops, free placement and more. It's a separate mod, loaded after RA, with RA unchanged from the Workshop: none of RA's files are in this repo, and nothing of RA's is edited or bundled. It isn't made by, or affiliated with, RA's authors.

**Load order:** `@CF;@Dabs;@RA_BaseBuilding;@DevZ_RAPatch`, with admin tools such as COT after CF. Server and players all load it. A server with this mod and no RA refuses to start.

## Settings

`profiles\DevZ_RAPatch\Settings.json` is written on first run, on the server only. Players' games never get the file. They're sent, in memory, only what their game needs: desk rules and modules, costs, tools, height limit, dismantling rules, recipes and the bag rules. Raid numbers, raid hours, decay, no-build zones, cooldowns, penalties and test settings stay on the server.

Most numbers start as RA's own. Worth a look, because they're ours: the gunpowder recipe, the workbench's ammo recipes, respawning at bags and desks, door and gate limits, material drops, and `Desk.DeployRule`. Restart the server after editing; players get the new values when they join.

### Decay

| Setting | Default | What it does |
|---|---|---|
| `Enabled` | 1 | 0: nothing decays and no nails are used |
| `FrameHours`, `WoodHours`, `MetalHours`, `ConcreteHours` | 2, 12, 18, 24 | Hours for a part with no upkeep to go from full health to gone |
| `DeskHours`, `ContainerHours` | 24, 24 | The same for the desk, and for tents, barrels and crates |
| `PartNailsPerDay`, `LadderNailsPerDay` | 0.5, 0.001 | Nails a day per part in a desk's radius |
| `DeskNailsPerDay`, `ContainerNailsPerDay` | 50, 0.00001 | The same for the desk itself, and containers |

Upkeep can't be exactly 0: the patch keeps it at 0.00001 or more, which is what RA uses for "as good as free". With `Enabled` 0, the desk still shows a time left, but nothing comes of it.

### Desk

| Setting | Default | What it does |
|---|---|---|
| `Radius` | 36 | Metres, measured in 3D |
| `MaxPlayers` | 0 | Authorised players. 0 = no limit |
| `OpenAuthorization` | 1 | 0: on a desk with no locked code lock, only the first player can authorise |
| `Modules` | empty | Items that upgrade a desk (see below) |
| `ModuleInstallSeconds` | 5 | How long installing or removing one takes |
| `ModulesRemovable` | 1 | 0: a module stays until the desk is gone |
| `RadiusMayOverlap` | 0 | 1: a radius module may overlap another desk's territory |
| `ModuleCraftDistance` | 5 | How close to the desk counts for a recipe's `NeedsModule`, in metres |
| `DeployRule` | 0 | Deploying inside a territory. 0 only its team, 1 anyone, 2 only its team, except traps and mines, which is RA's own way. RA's charges can always be placed, and building parts have their own rules |
| `DeployChecksPlacedSpot` | 1 | RA checks where the player stands, so someone just outside could place inside. This also checks where the object lands |
| `DashboardEnabled` | 1 | Authorised players can open the HQ dashboard on a desk |
| `MaxDoors` | 8 | Single and double doors per territory, shown on the dashboard. 0 = no limit |
| `MaxGates` | 2 | Compound gates and garage doors per territory. 0 = no limit |
| `RaidAlarmCooldownSeconds` | 300 | The most often a desk posts to Discord. Hits in between are counted in the next message |

**Modules** can be any item class. You name them here, then decide how players find them in your own loot tables or trader. Hold the item, look at the desk, and choose Install. It's used up, and comes back if it's removed. The dashboard has a Remove button per module, and empty hands on the desk offers Remove for the last one in.

```json
"Modules": [
  { "Name": "Workbench", "Items": ["AmmoBox"], "RadiusAdded": 0, "PlayersAdded": 0, "UpkeepMultiplier": 1, "Storage": 0 },
  { "Name": "Zone", "Items": ["BaseRadio"], "RadiusAdded": 20, "PlayersAdded": 2, "UpkeepMultiplier": 1, "Storage": 0 },
  { "Name": "Storage", "Items": ["SmallProtectorCase"], "RadiusAdded": 0, "PlayersAdded": 0, "UpkeepMultiplier": 1, "Storage": 1 }
]
```

- Each row can do any mix of these, up to 16 rows. `Items` can list several classes, and anything inheriting from one counts.
- One of each per desk. The authorised players can install and remove.
- A container has to be empty first, so nobody loses gear.
- `Storage` opens 10 x 10 of space on the desk. The size is set in config, in `generate_config.py`. A module can't be taken out, and the desk can't be packed up, while there's something in it.
- If the desk is destroyed or packed up, its modules drop beside it.
- `"RaidAlarm": 1` makes a module a raid alarm. Its team adds their own Discord webhook on the dashboard, and gets an alert when an enemy damages anything in the territory: melee, gunfire, grenades or charges, with the base's map position. Zombies, animals, decay and the team's own hits don't count, though RA's charges don't say who armed them, so a team's own charge will alert. Only real Discord webhook addresses are accepted, and they stay on the server, saved with the desk.
- A recipe with `NeedsModule` set to a module's `Name` only works near a desk with that module, for players authorised on it.
- The server log warns at boot about any class name that doesn't exist. They're easy to mistype.
- Changing a module's `Name` on a live world loses that module from its desks. Modules are saved by name.

### Respawn

Where a dead player comes back. There's one way back, not a chain:

1. **Bags on:** their sleeping bag, if they've set one and it's off cooldown. The desk isn't used.
2. **Bags off, desks on:** a Territory HQ they're authorised on, if it's off cooldown.
3. **Anything else,** including a cooldown: vanilla's spawn points. A message says when their bag or desk is ready.

| Setting | Default | What it does |
|---|---|---|
| `BagsEnabled` | 1 | 0: no bag respawning, so desks are used instead |
| `HQEnabled` | 1 | Only matters with bags off. 0 as well: vanilla's spawn points |
| `BagClasses` | our bag | Laid-out objects that count as a sleeping bag: any mod's class, or anything inheriting from it |
| `BagCooldownSeconds`, `HQCooldownSeconds` | 1800, 3600 | Per player, and restarts don't clear them |
| `CooldownPenaltySeconds` | 600 | Respawn again within the window, and the next cooldown is this much longer. It adds up. 0 = off |
| `CooldownPenaltyWindowSeconds` | 1800 | How long after a cooldown ends a respawn still counts. Wait longer, and it's back to the base cooldown |
| `CooldownMaxSeconds` | 7200 | The longest a cooldown can grow to. 0 = no limit |
| `RespawnWaterPercent`, `RespawnEnergyPercent` | 50, 50 | Water and energy to come back with at a bag or desk. Health and blood are full |
| `BagWearPercent` | 10 | Of the bag's health, per respawn. It stops working when ruined. 0 = no wear |
| `BagsNeedOwnTerritory` | 0 | 1: a bag can only be laid out inside a territory the player is authorised on. It also only works while it stays in one: if they're taken off the desk, or the desk is gone, it's skipped and they get vanilla's spawn points. The outline goes red and the action says why, and the server checks the spot where the bag lands too. Any bag in `BagClasses` that's laid out with an outline is covered |
| `BagPickup` | 2 | Our bag only. 0 anyone, 1 only who laid it out, 2 any team member: who laid it out, or anyone authorised on the desk it's in |
| `BagLifetimeHours` | 336 | How long our laid-out bag lasts, when it's not moved or used |

**The penalty, with the default numbers:** the bag's cooldown is 30 minutes. Respawn on it within 30 minutes of that ending, and the next cooldown is 40. Do it again and it's 50, up to 2 hours. Stay away for more than 30 minutes after a cooldown ends, and it's back to 30.

**Our bag:** `RAPatch_SleepingBag_Black`, `_Blue`, `_Green`, `_Grey` and `_Red`. Hold it, raise the hologram, place it, and look at it: "Set or clear respawn here". A player has one bag at a time, and a whole team can set the same one. Inside a territory, only its team can set a bag. Whether a bag can be laid out in someone else's follows `Desk.DeployRule`, unless `BagsNeedOwnTerritory` is on: that's stricter, and refuses both someone else's territory and open ground. Where desks overlap, the oldest owns the spot. It only limits laying out: a bag already down stays usable if its desk goes.

**Another mod's bag:** put its laid-out class in `BagClasses`, such as `ZenSleepingBagDeployed_Base` for Zen's, and take ours out if you don't want it. You don't need ours or any change to that mod. The catch is that `BagPickup` and laying out only cover our own bag, because another mod's bag has its own actions. What a player has set is saved in `RespawnData.json` as the object's class and position, so moving or packing a bag clears it.

**Other mods that handle spawning**, such as Expansion's Spawn Selection, may fight with this. Switch one off.

The bag's model and textures are Zenarchist's: see Credits and licence below.

### Vehicle locks

A code lock on any car, including other mods' cars. Motorbikes and boats aren't covered: DayZ makes them separate classes.

| Setting | Default | What it does |
|---|---|---|
| `Enabled` | 1 | 0: no car can be locked, and fitted locks do nothing. Turn it off beside another mod's car locks or keys |
| `LockItem` | `RA_CodeLock` | What's fitted: any item class, or anything inheriting from it |
| `RemoveTool`, `RemoveSeconds` | `Pliers`, 15 | Takes the lock off, for someone who has entered the code. The item comes back |
| `BreakTool`, `BreakSeconds` | `Lockpick`, 60 | Forces a locked car. Empty `BreakTool` = it can't be forced |
| `BreakChance` | 0.3 | From 0 to 1. If it works, the lock's lost |
| `BreakToolWearPercent` | 34 | Of the tool's health, per try |
| `AlarmOnBreak` | 1 | The car's horn pulses, half a second on and half off, while someone's forcing it |
| `WrongCodeWaitSeconds` | 5 | A wait after a wrong code, so a code can't simply be run through |
| `PlacementAdmins` | empty | Steam64 IDs who get the placement tool, and "[ADMIN] Forget my code" for testing a car as someone without the code |
| `Spots` | vanilla cars and two UAZ mods | Where the lock is drawn on each car class. The placement tool writes these. A car with no row of its own uses its nearest relative's, so one colour covers the others |

**Using it:** hold the lock item, look at a car with nobody in it, and choose "Fit code lock": a quick attach, like a lock on a door. It's used up. "Set code" opens RA's keypad. From then on there's nothing to lock or unlock: whoever has the code uses the car as normal, and everyone else is shut out of getting in, its doors, hood and trunk, its parts and its inventory. So a player shot beside their car hasn't left it open. Someone without the code gets "Enter code", and is allowed from then on. To reset who's allowed, take the lock off and fit it again. Anyone already inside can still get out.

**What you see:** an RA code lock, drawn by each player's own game and fixed to the car's body, so it doesn't matter what happens to the doors. It's only a picture, so a badly placed one can't break anything, and a car with no saved spot gets a first guess on its left side.

**Limit:** the inventory is shut on each player's own game, and the actions are also checked on the server. Moving items between inventories isn't checked again on the server, so a hacked game could get past that part.

**The placement tool:** add your Steam64 ID to `PlacementAdmins`. Look at a car with a lock on it, with empty hands: "[ADMIN] Place code lock". Nudge it with the buttons and save. It's saved for every car of that class and everyone online sees it at once, with no restart. Reopen the tool after moving round the car to check another angle.

### Build

| Setting | Default | What it does |
|---|---|---|
| `Costs` | empty | Rows of `Part`, `Item`, `Quantity`, `Multiplier` (see below) |
| `RepairCostMultiplier` | 1 | |
| `RepairBelowPercent` | 95 | Repair is only offered below this much health, so tiny decay doesn't offer a paid repair. 100 = any damage, as RA has it |
| `RepairCostByDamage` | 1 | Pay for the damage, at least 1. 0 = RA's own sum, full cost times the health left, which charges the least damaged part the most |
| `UpgradeTools` | claw hammer 1 s, hatchet 6 s, hammer 3 s | Rows of `Tool` and `Seconds`, for upgrading and repairing. A tool not listed can't do either. `SledgeHammer` can be added |
| `HeightLimitMeters` | 25 | Above the ground |
| `BuildAnywhere` | 0 | 1: parts ignore other objects when placed |
| `ServerChecksSpot` | 0 | 1: the server repeats the height and no-build checks. RA leaves those to the player's machine |
| `NoBuildZones` | empty | Rows of `X`, `Z`, `Radius` |
| `BlueprintFromPaper`, `BlueprintSeconds` | 1, 3 | Turning paper into a blueprint |
| `FreePlacement` | 0 | 1: parts that only snap can also be placed anywhere, and turned. A wall can stand on its own, even in mid-air. Snapping still wins when there's something to snap to. Doors and gates still need a frame |
| `SlowRotationMultiplier` | 0.2 | Hold Shift while turning a part to turn this much slower. 1 = no difference |

A `Costs` row's `Part` is a class name such as `BaseBuilding_WoodWall`, or `Tier:Frame`, `Tier:Wood`, `Tier:Metal`, `Tier:Concrete` or `Tier:All`. The closest match wins. Leave `Item` empty and `Quantity` 0 to keep RA's. `Quantity` -1 means that part can't be built. The lowest possible cost is 1: RA reads "no cost" as "can't be built".

```json
{ "Part": "BaseBuilding_WoodWall", "Item": "Nail", "Quantity": 3, "Multiplier": 1 },
{ "Part": "Tier:Metal", "Item": "", "Quantity": 0, "Multiplier": 2 }
```

### Raiding

| Setting | Default | What it does |
|---|---|---|
| `C4Damage`, `ImprovisedDamage` | 1275, 750 | Health taken off each part the blast hits |
| `C4FuseSeconds`, `ImprovisedFuseSeconds` | 58, 58 | |
| `DestroyedMaterialsPercent` | 25 | Of a part's cost, dropped when it's destroyed by anything, including decay. 0 = nothing. -1 = RA's own, a coin flip and then a random amount |
| `LogCharges` | 1 | Who armed and disarmed each charge, and when it went off, with time, Steam64 ID, position and territory, in `profiles\DevZ_RAPatch\ChargeLog.txt`. Admins only: never sent to players |
| `BlastRadiusMeters` | 0.1 | How far the blast looks for parts. RA's is very short: the charge has to touch |
| `PenetrationDepth`, `PenetrationFalloff` | 0, 0.65 | Parts the blast goes through, and damage kept per part |
| `C4PlayerAmmo`, `ImprovisedPlayerAmmo` | RA's | The vanilla explosion made for players and sound. `Plastic_Explosive_Ammo` hurts more |
| `FrameDamageMultiplier`, and `Wood`, `Metal`, `Concrete` | 1 | The blast's damage against each tier |
| `DoorsOnly` | 0 | 1: only doors, gates, hatches, windows and shutters take blast damage |
| `BlastProofClasses` | empty | Classes that take no blast damage, with anything inheriting from them |
| `MeleeDamage`, `BulletDamage`, `OtherExplosiveDamage` | 1, 1, 1 | Whether these can hurt parts. The last doesn't cover RA's charges |
| `RaidHoursEnabled`, `RaidHoursUtcOffset`, `RaidWindows` | 0, 10, Fri-Sun | When on, charges can only be armed in these windows. A lit fuse still finishes |

### Dismantling

| Setting | Default | What it does |
|---|---|---|
| `ToolRemoveEnabled` | 1 | The hammer or hatchet's Remove on RA's parts |
| `RemoveWindowSeconds` | 600 | How long after being placed or upgraded a part can be removed or rotated |
| `VanillaSecondsAuthorized`, `VanillaSecondsOutsider` | 5, 300 | Dismantling vanilla parts such as a watchtower |
| `VanillaOutsidersAllowed` | 1 | 0: outsiders can't dismantle vanilla parts at all |
| `DeskPackUp` | 0 | Screwdriver or pliers on a desk with no lock. 0 = anyone (RA), 1 = authorised players, 2 = nobody |
| `DeskPackUpSeconds` | 2.5 | |

### Crafting

`DeskKit` is RA's own recipe for the desk. `Recipes` holds up to 12 of our own. Each has `Name`, `Enabled`, `Ingredient1` and `Ingredient2` (lists of classes, anything inheriting counts), `Ingredient1Amount` and `Ingredient2Amount` (0 = only has to be there), `Ingredient1Consumed` and `Ingredient2Consumed`, `Result`, `ResultQuantity`, `ResultOnGround`, `NeedsModule` and `Seconds`.

**The defaults:**
- Gunpowder: pliers in hand plus one full stack of any ammo gives 1 gunpowder, 8 s. The pliers aren't used up. It's a straight trade with the workbench, where 1 gunpowder makes 1 full stack of any ammo.
- Improvised charge: 5 gunpowder plus 25% duct tape, 12 s. Off by default, because the workbench makes these. Set `Enabled` to 1 to use it as well.

Set `Enabled` to 0 to remove one. A recipe whose `Result` isn't an item on this server switches itself off and logs a warning, so a recipe for another mod's item is safe to leave in. One ingredient has to be in the player's hands, as with all vanilla crafting.

### Workbench

A module with `"Workbench": 1` in its row adds "Open Workbench" to the desk, for authorised players. The menu lists the recipes in `Crafting.Bench.Recipes`, with any number of ingredients and no recipe limit. A recipe for items this server doesn't have doesn't show, and the server log lists those at startup.

- **Paying:** ingredients are taken when Craft is pressed, from the items directly in the desk's storage first, then the player's inventory. A box inside storage isn't opened, and nails in the upkeep slot are never touched.
- **Time:** the timer runs on the desk, so the player can walk away, and restarts don't lose it.
- **Finished items** wait on the desk until someone authorised presses Collect. Whatever doesn't fit lands on the ground beside it.
- **Cancelling** takes out the last job, and gives back what it took.
- **Packed up or destroyed:** finished items and paid-for materials land beside the desk. The module can't be removed while the bench has anything in it.

| Setting | Default | What it does |
|---|---|---|
| `QueueSize` | 5 | Jobs on one desk, including the one being made |
| `MaxWaiting` | 20 | Finished items a desk can hold before a new job is refused |
| `Recipes` | RA's, and every gun's ammo | `Name`, `Enabled`, `Seconds`, `Ingredients` (`Items`, `Amount`, `Consumed`), `Results` (one at random), `Count`, `QuantityPercent` |

The default recipes copy the numbers in Rearmed's server pack, as data, except ammo. RA's 1 gunpowder for 2 full stacks would make ammo from nothing with our gunpowder recipe, so all ammo is 1 gunpowder for 1 full stack, 40 s. That makes ammo and gunpowder a straight trade. Many of their items only exist with that pack, so on a server without it those recipes stay hidden until you change them.

### Not in the file

- **Part health:** `part_health.csv` (`Class`, `Hitpoints`), then rebuild. The game reads health from config before any script runs. Players need the new build.
- **Snap distance and camera distance:** `RAPatch_Constants.c`, then rebuild. Only a compiled value works for these.
- **Loot:** `extras\types.xml`. Copy it to a mission folder and add it through `cfgeconomycore.xml`. The numbers are a starting point. RA's claw hammers are `RA_ClawHammer_Blue`, `_Red` and `_Yellow`: plain `RA_ClawHammer` can't spawn.

## Testing a change

`SelfTest` 1 in the settings file makes the server spawn real RA objects at `SelfTestX`, `SelfTestZ` on boot, check them against the file, remove them, and write `profiles\DevZ_RAPatch\SelfTest.txt`. Don't leave it on, and don't use it on a real world: it uses one spot on the map and leaves a test character there.

Join through the DayZ launcher. Starting `DayZ_x64.exe` directly gets you kicked with "Game restart required" within a minute, with or without this mod.

## Credits and licence

- **RA Base Building** is by its own authors, and isn't part of this mod. Players and servers download it from the Workshop.
- **Sleeping bag:** the models and textures in `src\data\sleepingbag\` are Zenarchist's, from [ZenSleepV2](https://github.com/ZenarchistCode/ZenSleepV2) and [Zen's Sleeping Mod](https://steamcommunity.com/sharedfiles/filedetails/?id=3468961047). His readme lets anyone repack and use them. They keep his terms, not this mod's licence: see `CREDIT.txt` there.
- **Workbench recipes:** the default numbers follow Rearmed's server pack, except ammo. No code, art or layouts of theirs are used.
- **Everything else** is DevZ's own, under the MIT licence: see [LICENSE](../LICENSE).
