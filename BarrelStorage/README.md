# DevZ Barrel Storage

A base-building rack that holds **up to six barrels**. Place the kit like a fence kit, then put
barrels on the rack; they still open and close normally, so you can reach the loot inside or let
them catch rain.

This is **Xantarn's [INVICTUZ_Barrel_Storage](https://steamcommunity.com/sharedfiles/filedetails/?id=3406014628)**
(Workshop 3406014628), fixed for DayZ 1.30. The rack, the kit, their models and textures are
Xantarn's work, and Xantarn gave permission to do anything with the mod. DevZ fixed the scripts and the
config.

- **Players need it too** (it's a normal mod, not a server-only one).
- **Needs Community Framework (CF)**, for the kit's carry animation. Load it after CF.
- **Use it instead of the original, never both.** It uses the same item names, so they'd clash.

## Switching from the original

The item and slot names are the original's (`Invictuz_Barrel_Storage`, `BarrelStorage_Kit`,
`BarrelStorage_Holo`, `SlotBarrel1`-`6`), on purpose:

- **Racks already placed on your server stay**, with their barrels on them.
- **Your types.xml entries keep working**: nothing to rename.

Take the original out of `-mod=`, add this one, and restart.

## What was fixed

| Problem on DayZ 1.30 | Fix |
|---|---|
| **Placing the kit desynced**: the kit deleted itself in the middle of the place action, which 1.30 moves to the ground just before | The kit now works like the vanilla base-building kits (fence, shelter): it stays in your hands while you place it, spawns the rack, hides itself, and the action removes it when it ends |
| **Garden plots (and other items) built as holograms**: the mod replaced the placement preview code for every item in the game | It no longer overrides placement at all. The kit just names its own preview (`projectionTypename`), the vanilla way |
| **Barrels on the rack looked open** to players while they were closed on the server | Vanilla opens a barrel the moment it's attached (meant for the truck bed), and players' games re-ran that whenever the rack came back into view. Now they keep the open/closed state the server sends |
| **The dismantle recipe** set its result on the wrong slot | A screwdriver on an empty rack dismantles it and drops the kit on the ground |
| **The carry animation** was set on the rack, which can't be held | It's set on the kit, so you carry it in both arms like a wooden crate |

## The deliberate change: vanilla barrel rules

The original let you pick up and carry **full** barrels, and put barrels inside barrels. This
version puts the **vanilla rules back**:

- A barrel must be **empty** (and closed) to pick it up or put it in cargo. You can't carry a
  barrel full of loot around, or nest barrels inside barrels.
- **Barrels on the rack still open and close normally**, to reach the loot or catch rain.
- To move a barrel on or off the rack, empty it, take it in your hands, and **attach** it to the
  rack (or **detach** it from the rack into your hands).

## Using it

1. Find a **Barrel Storage Kit** (loot) and take it in your hands.
2. **Place** it like a fence kit: a preview shows where the rack goes; confirm to build it.
3. Put up to six barrels on it: an empty barrel in hands, look at the rack, **Attach**.
4. To take it down: remove every barrel, then use a **screwdriver** on the rack (**Dismantle**).
   The kit drops on the ground.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 BarrelStorage -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_BarrelStorage`** into your server folder, add it to `-mod=` after CF, and
   copy its key from `keys\` into your server's `keys\`.
3. Add the two entries from [`types.xml`](types.xml) to your mission's `db\types.xml` (if you're
   switching from the original, you already have them).
4. Players get the same copy (Workshop or your server pack).

## Settings

No settings file. What you might change is in `types.xml`:

- **Rack lifetime: 3888000 seconds (45 days).** How long a placed rack stays when nobody touches
  it (Xantarn's file had 1200900, about 14 days). Long enough for a base that's visited now and
  then; lower it if abandoned racks pile up.
- **Kit loot:** `nominal` 10, `min` 2, found in towns, villages, farms, industrial, coast and
  hunting spots, tiers 1 to 3.

## How it works

Code: `src\scripts\4_World\`

| File | What |
|---|---|
| `config.cpp` | The rack, its preview, the kit, the six barrel slots and their spots in the rack's model; adds those slots to every barrel |
| `BarrelStorageKit.c` | The kit: a base-building kit that spawns the rack where the preview was |
| `BarrelStorage.c` | The rack (can't be picked up, has no cargo of its own) and its preview |
| `Barrel.c` | Adds attach/detach to barrels, and keeps a rack barrel's open/closed state on players' games |
| `Recipes\` | The screwdriver dismantle recipe |
| `Animations.c` | The kit's carry animation (CF's animation register) |

**Why the PBO prefix is still `Invictuz_Barrel_Storage`:** the models point at their textures
and at the six barrel spots by that folder name, inside the model files. Renaming the folder
would leave the rack without barrel spots.

## Testing it

1. Spawn a `BarrelStorage_Kit`, place it: the rack appears where the preview was, the kit is
   gone, and nothing desyncs.
2. Place a garden plot: it builds as a real plot, not a hologram.
3. Put a barrel on the rack, fill it, close it, walk away until the rack is out of view and come
   back: the barrel still shows closed.
4. Try to pick up a full barrel: you can't. Empty it and you can.
5. Remove every barrel, screwdriver on the rack: you get the kit back on the ground.

## Credits and licence

- **Xantarn**: the original INVICTUZ_Barrel_Storage (models, textures, the idea and the first
  scripts). [Workshop page](https://steamcommunity.com/sharedfiles/filedetails/?id=3406014628).
  Used with Xantarn's permission to do anything with it.
- **DevilinJake**: the 1.30 fixes.

The DevZ changes are MIT, like the rest of this repo.
