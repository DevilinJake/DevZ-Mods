"""Writes each mod's Workshop preview image and description into <Mod>\\workshop\\.

  python tools\\make_workshop.py

preview.png goes in Publisher's preview image box, description.txt (Steam BBCode) in its
description box. Edit the MODS table below, then re-run. The card itself is drawn by
workshop_preview.py (needs Pillow and numpy).
"""
from pathlib import Path

from workshop_preview import card

REPO = Path(__file__).resolve().parent.parent
GITHUB = "https://github.com/DevilinJake/DevZ-Mods"

# folder: (title on the image, accent colour, one-line summary, what it does, defaults)
MODS = {
    "InventorySizes": (
        "Inventory\nSizes", (86, 156, 214),
        "Bigger cargo for clothing, backpacks, vests and pouches. Ready to use as it is, or build your own sizes (see below).",
        [
            "Overrides only one value per item ([b]itemsCargoSize[/b]): no scripts, no performance cost.",
            "Every grid is one row in a table, so all colour versions of an item change together.",
        ],
        [
            "Most items get a little less than double their vanilla space (typically 50-90% more).",
            "The largest backpacks go further (Field, Long Haul, Tactical, Mountain, up to 10x16), so they carry sheet metal, barrels and vehicle parts.",
        ],
    ),
    "AmmoStacks": (
        "Ammo\nStacks", (214, 157, 70),
        "Bigger loose-ammo stacks: 2x out of the box, or build your own multiplier and per-ammo sizes (see below).",
        [
            "Overrides only one value per ammo type ([b]count[/b]): no scripts, no performance cost.",
            "Weight per round doesn't change: a stack of 50 weighs the same as two of 25.",
        ],
        [
            "2x stacks: 9x19 goes from 25 to 50, 12ga from 10 to 20, .22 from 50 to 100.",
            "Single-round items (rockets, 40mm grenades, darts) stay at 1.",
            "Loot: run [b]fix_loot.py[/b] from the GitHub repo once on your types.xml so the ammo found in the world stays the same size. Skip it if you want more ammo in the world.",
        ],
    ),
    "WeaponDurability": (
        "Weapon &\nSuppressor\nDurability", (196, 92, 92),
        "Weapons and suppressors wear out 10x slower out of the box, or build your own multipliers and per-weapon values (see below).",
        [
            "Every shot wears a gun by the ammo's damageBarrel divided by the gun's [b]barrelArmor[/b]. This mod only raises barrelArmor, so 10x armor = 10x as many shots.",
            "No scripts, no performance cost. Safe to add to a live server.",
        ],
        [
            "Weapons last 10x as long.",
            "Suppressors and other muzzle attachments last 10x as long. Exception: the plastic bottle (improvised) suppressor lasts 3.3x, so it stays a stopgap.",
        ],
    ),
    "CombineNoSplit": (
        "Combine\nKits", (106, 176, 110),
        "Combine part-used bandages, repair kits and gas mask filters, but never split them.",
        [
            "Drag a 2-use bandage onto a 3-use one: you get a full bandage (4 uses) plus one with 1 use. A 60% sewing kit onto another 60% kit gives 100% + 20%.",
            "Items can never be split, so nobody turns one 4-use bandage into four 1-use bandages.",
        ],
        [
            "Bandage, Duct Tape, Electrical Repair Kit, Epoxy Putty, Gas Mask Filter, Improvised Gas Mask Filter, Leather Sewing Kit, Sewing Kit, Tire Repair Kit, Gun Cleaning Kit, Sharpening Stone.",
        ],
    ),
    "RemoveRecipes": (
        "Remove\nRecipes", (170, 110, 200),
        "Switch off right-click crafting recipes from vanilla or any mod, without editing that mod. You pick the recipes and build your own copy (see below).",
        [
            "Handy when a mod has a shortcut recipe that skips its own crafting stations or rare ingredients.",
            "A scanner lists every right-click recipe in the game and your mods; tick the ones to remove.",
            "Bench/station recipes (e.g. Crafting Plus) live in those mods' own settings; this mod is for right-click recipes.",
        ],
        [
            "[b]None.[/b] This Workshop build removes nothing: it's a tool you customise for your server (see below).",
        ],
    ),
    "RepairToPristine": (
        "Repair To\nPristine", (90, 190, 190),
        "Repair kits repair items all the way to Pristine instead of stopping at Worn.",
        [
            "Each repair still moves an item up one condition level and uses some of the kit, so Worn to Pristine costs one more repair.",
            "Ruined items stay unrepairable, the game's way.",
        ],
        [
            "Every vanilla kit: Gun Cleaning Kit, Sewing Kit, Leather Sewing Kit, Duct Tape, Sharpening Stone, Epoxy Putty, Electrical Repair Kit, Tire Repair Kit, Blowtorch. Kits built on these count too.",
        ],
    ),
    "AutoStack": (
        "Auto\nStack", (214, 190, 80),
        "Loose ammo merges into your existing stacks automatically. Server-side only.",
        [
            "Pick up 12 rounds of 9x19 while carrying a stack of 40: you end up with one stack of 52, not two.",
            "Works in your clothing and backpack, and in containers (crates, tents, car trunks, dropped backpacks). Ammo in your hands is left alone.",
            "Each round keeps its own condition; ruined ammo is never merged.",
        ],
        [
            "Merging on in player inventories and in containers.",
            "Settings file: [b]<profiles>\\DevZ_AutoStack\\Config.json[/b], created on first start. Change it with the server stopped; no rebuild needed. If inventories \"jump\" while dragging ammo, raise [b]DelayMs[/b].",
        ],
    ),
    "InfectedDamage": (
        "Infected\nDamage", (150, 170, 80),
        "Zombies hit 1.5x harder out of the box, or build your own multipliers for Health, Blood, Shock, bleed chance and clothing wear (see below). Server-side only.",
        [
            "One multiplier each, plus per-attack overrides (soldier, police, mummy zombies...).",
            "Config values only: no scripts, no performance cost.",
        ],
        [
            "Health 1.5x: 11 swipes kill a player with no armour (vanilla 16).",
            "Shock 1.5x: a group can knock you out with quick hits.",
            "Bleed chance 1.5x: 16.5% per hit (vanilla 11%).",
            "Blood and clothing wear unchanged.",
        ],
    ),
    "QuickTransfer": (
        "Quick\nTransfer", (110, 170, 230),
        "With the inventory open, hold a key and sweep the mouse over items: each one moves between your inventory and the nearest open container.",
        [
            "Items in your clothing and backpack go to the nearest container you have open in vicinity (crate, tent, car trunk, barrel...). Items in a container or on the ground come into your inventory.",
            "Only moves what fits. Nothing you wear or hold is moved, and an item never bounces back while you keep the key down.",
            "Every move is a normal inventory move, so the server checks it exactly like a drag and drop.",
        ],
        [
            "Key: [b]Left Alt[/b]. Change it in Options > Controls, under [b]DevZ Mods[/b].",
            "No settings file: there's nothing for admins to set.",
        ],
    ),
    "CrashSafety": (
        "Crash\nSafety", (226, 120, 70),
        "Vehicle crashes hurt and knock out the crew instead of killing them, and leave the vehicle badly damaged instead of ruined. Server-side only.",
        [
            "The crew lose 10-70% health and the worst crashes knock them out for up to 45 seconds. A crash never kills.",
            "The hull, engine and fuel tank (the parts that can't be replaced) stop at 10% (Badly Damaged) from crashes, falls, fire, over-revving, no coolant or deep water, so the vehicle can always be repaired. Rotors, wheels, doors and other parts still take full damage and can be ruined.",
            "Expansion helicopters no longer burn out from a slow tip-over on landing.",
        ],
        [
            "Hull, engine and fuel tank floor 10%; guns and explosives can still destroy vehicles (set [b]FloorsAgainstWeapons[/b] to 1 to stop that too).",
            "Settings file: [b]<profiles>\\DevZ_CrashSafety\\Config.json[/b]. Change it with the server stopped; no rebuild needed.",
        ],
    ),
    "AutoRun": (
        "Auto\nRun", (120, 200, 170),
        "Press a key and keep moving forward at the speed you have: walking, jogging or sprinting. Steer with the mouse. Any movement key stops it.",
        [
            "Keeps the speed you had when you pressed the key (jog if you were standing still). Sprint ends with your stamina and comes back with it, as it does when you hold Shift.",
            "Stops on W, A, S, D, the key again, death, being knocked out, being restrained, getting into a vehicle, and menus you can't walk in (Esc, traders).",
            "Doesn't stop for jumping, vaulting, crouching or going prone, the inventory, the map or typing in chat: you carry on, the way you would with W held.",
            "Multiplayer-safe: the server is told the same state as your game, so you're never pulled back.",
        ],
        [
            "Key: [b]Z[/b]. Change it in Options > Controls, under [b]DevZ Mods[/b]. (On a keyboard, vanilla doesn't use Z. Running Expansion? Its own auto-run is on Z too: switch one of them off.)",
            "No settings file: there's nothing for admins to set.",
        ],
    ),
    # folder, PBO and settings path are still HouseRaidRules / DevZ_HouseRaidRules (its old name)
    "HouseRaidRules": (
        "Ninjins\nHouse Raid Fix", (222, 104, 140),
        "Rebuilt house walls, doors, windows and barricades follow the same PvE and raid-hour rules as your fences and gates. Server-side add-on for Ninjins PvP/PvE Upgraded.",
        [
            "Ninjins Raid Extension stops bullets and explosives on base-building [i]items[/i] outside your raid zones. A house rebuilt with bricks, mortar and a trowel (Nasdara and other maps with vanilla rebuilding) isn't an item: its rebuilt wall is part of the house, so it could be shot or blown open in a PvE zone while the fence next to it couldn't. This mod puts the same check on the house.",
            "Zones, the admin flag and the on/off switches are read from Ninjins at runtime. Nothing of Ninjins' is copied or changed.",
            "Raid hours come from Expansion's RaidSettings.json schedule when Expansion Base Building is loaded, otherwise from the mod's own schedule.",
        ],
        [
            "Hits land only in PvP and Raid zones, and only during raid hours (Friday 18:00 to Sunday 21:00 server time unless Expansion's schedule applies). Safe zones and PvE: no damage. Melee hits are held to the same rules.",
            "Follows Ninjins Raid Extension's Enabled, BlockBulletDamage, BlockExplosiveDamage and BypassForAdmins, so there's one place to edit.",
            "Settings file: [b]<profiles>\\DevZ_HouseRaidRules\\Config.json[/b]. Change it with the server stopped; no rebuild needed.",
        ],
    ),
    "SatchelSoundFix": (
        "Satchel\nSound Fix", (230, 110, 100),
        "Fixes a DayZ Expansion bug: after a satchel charge explodes, its arming beep keeps squealing at the crater for minutes. With this mod the beeping stops with the bang.",
        [
            "Expansion's satchel keeps a per-frame beep job running on every player's game after the explosion, and by then the fuse time is 0, so a new beep plays every frame. This mod removes that job the moment the explosion arrives, and never lets a 0-second fuse beep every frame.",
            "Expansion's own explosion (bang, particles, damage) is untouched: the fix runs after it.",
            "Needs [b]DayZ Expansion[/b] Base Building and must load after it. Without Expansion the mod compiles to nothing and does no harm. No Expansion files are copied or changed.",
        ],
        [
            "No settings file: there's nothing for admins to set.",
        ],
    ),
    "HouseCodeLock": (
        "Expansion\nHouse CodeLock", (200, 160, 90),
        "Fit the DayZ Expansion code lock to rebuilt vanilla house doors. A stopgap until Expansion supports house doors itself. Needs DayZ Expansion (Base Building).",
        [
            "Code lock in hands, looking at a closed, rebuilt house door: [b]Attach[/b]. It goes in the same spot as the vanilla digital code lock.",
            "Then the Expansion keypad, as on a gate: [b]Set code[/b], [b]Close & lock[/b], [b]Enter code[/b], [b]Unlock[/b] (players who know the code), [b]Change code[/b].",
            "A locked door can't be opened, and a lockpick can't lock or unlock it. A ruined lock no longer holds the door, like the vanilla locks.",
            "Expansion's own server settings apply: code length, remembering players who know the code, territory members, damage for a wrong code, raid logs.",
            "Codes and lock states survive restarts.",
        ],
        [
            "All 10 door spots on every rebuildable house, wooden and metal doors.",
            "No settings file: it follows your Expansion BaseBuilding settings.",
        ],
    ),
    "BarrelStorage": (
        "Barrel\nStorage", (176, 132, 82),
        "A base-building rack that holds up to six barrels. This is [b]INVICTUZ_Barrel_Storage by Xantarn[/b] ([url=https://steamcommunity.com/sharedfiles/filedetails/?id=3406014628]Workshop 3406014628[/url]), which broke on DayZ 1.30, fixed and with vanilla barrel rules. Xantarn gave permission to do anything with it.",
        [
            "Place the kit like a fence kit, then attach up to six barrels to the rack. Barrels on the rack still [b]open and close normally[/b], to reach the loot or catch rain. Screwdriver on an empty rack: you get the kit back.",
            "[b]Fixed: kit placement on 1.30.[/b] The kit deleted itself in the middle of the place action and desynced. It now works like the vanilla base-building kits (fence, shelter).",
            "[b]Fixed: garden plots building as holograms.[/b] The original replaced the placement code for every item in the game; this version doesn't override placement at all.",
            "[b]Fixed: barrels on the rack showing open[/b] to players while they were closed on the server.",
            "[b]Fixed: the dismantle recipe and the kit's carry animation.[/b]",
            "[b]Fixed: the rack faced away from you[/b] (placed against a wall, its barrels faced the wall). It now faces you.",
            "[b]Fixed: a second's freeze[/b] the first time a kit appeared. The models are now in the game's fast-loading format (the kit went from 35 MB to 3.7 MB).",
            "[b]Changed on purpose: vanilla barrel rules are back.[/b] A barrel must be empty (and closed) to pick it up or put it in cargo, so you can't carry barrels full of loot or put barrels inside barrels.",
            "Same item names as the original ([b]Invictuz_Barrel_Storage[/b], [b]BarrelStorage_Kit[/b]): switching from it keeps your placed racks and your types.xml entries. Use this one [b]instead of[/b] the original, never both.",
        ],
        [
            "Rack lifetime: we recommend [b]3888000[/b] (45 days) in types.xml, so a base that's visited now and then keeps its rack. The types.xml entries are on GitHub.",
            "Kit loot: nominal 10, min 2, in towns, villages, farms, industrial, coast and hunting spots, tiers 1 to 3.",
            "No settings file.",
        ],
    ),
}

# Server-side mods: loaded with -servermod, players never download them
SERVER_ONLY = {"AutoStack", "InfectedDamage", "CrashSafety", "HouseRaidRules"}


# What each mod's scanner finds in other mods, for the "Works with your other mods" section
SCANS = {
    "InventorySizes": "the backpacks, clothing, vests and pouches from any clothing or gear mod",
    "AmmoStacks": "the ammo from any weapon mod",
    "WeaponDurability": "the guns, suppressors and muzzle attachments from any weapon mod",
    "CombineNoSplit": "every item with uses or a percentage from any mod (medical, repair kits, filters, bottles)",
    "RemoveRecipes": "every right-click recipe from every mod you run",
    "RepairToPristine": "the repair kits from any mod (for example a multitool that works as a sewing kit)",
    "InfectedDamage": "the attacks of the zombies from any zombie mod",
    "AutoStack": None,  # needs no scanning: works with any ammo as it is
    "QuickTransfer": None,
    "CrashSafety": None,
    "AutoRun": None,
    "HouseRaidRules": None,
    "SatchelSoundFix": None,
    "HouseCodeLock": None,
    "BarrelStorage": None,
}

# For mods without a scanner: the "Works with your other mods" text
NO_SCAN = {
    "AutoStack": "Works with any ammo, vanilla or modded, and any stack size, including [b]DevZ Ammo Stacks[/b]. Nothing to set up.",
    "QuickTransfer": "Works with items and containers from any mod, and with bigger inventories such as [b]DevZ Inventory Sizes[/b]. Nothing to set up.",
    "CrashSafety": "Works with vanilla vehicles and modded ones built on them, including Expansion's helicopters and boats. Nothing to set up. Leave other mods' crew damage multipliers at 1.",
    "AutoRun": "Works on any map with any other mods: it changes no items and no settings. Nothing to set up.",
    "SatchelSoundFix": "Works alongside any other Expansion add-on: it only touches the satchel's beep. Nothing to set up.",
    "HouseCodeLock": "Needs [b]DayZ Expansion[/b] (the Bundle or Base Building): load it after Expansion in -mod=. Works on any map with rebuildable vanilla houses. Expansion's code locks on gates, walls, tents and safes work exactly as before. When Expansion adds house doors itself, remove this mod.",
    "BarrelStorage": "Works on any map. Barrels from other mods that are built on the vanilla barrel fit the rack too. Don't load it next to the original INVICTUZ_Barrel_Storage: they use the same item names.",
    "HouseRaidRules": "Works on any map with vanilla house rebuilding, next to Ninjins Raid Extension's own rules for fences, gates, towers and tents. With Expansion Base Building loaded, raid hours follow Expansion's RaidSettings.json, so the house parts open and close with Expansion's walls. The sledgehammer and dismantling are actions, not hits: block ActionDestroyPart and ActionDismantlePart in Ninjins Core's ActionsManager.json for your PvE and safe zones, as you would for any base.",
}

# Extra first lines for the "Server setup" section: what a mod needs loaded next to it
SETUP_EXTRA = {
    "BarrelStorage": ["Needs [b]Community Framework (CF)[/b] in -mod=, loaded before this mod.",
                      "Add the two types.xml entries (rack and kit) from the GitHub page to your mission's [b]db\\types.xml[/b]. Switching from the original? You already have them."],
    "HouseRaidRules": ["Needs [b]Ninjins PvP/PvE Upgraded[/b] in -mod= (its Core, Zones and Raid Extension), with this mod loaded after it; without Ninjins it has nothing to read and won't load. Expansion is optional."],
}


# The picture on each preview card (drawn in workshop_preview.py)
PICTOGRAMS = {
    "InventorySizes": "inventory",      # small pack -> big pack
    "AmmoStacks": "ammo",               # rows of cartridges, x2 badge
    "WeaponDurability": "weapon",       # suppressed AK, shield with a tick
    "CombineNoSplit": "combine",        # half + half -> full, splitting crossed out
    "RemoveRecipes": "recipes",         # a right-click recipe row, crossed out
    "RepairToPristine": "repair",       # condition dots red -> green, wrench, sewing kit
    "AutoStack": "autostack",           # two loose groups -> one stack
    "InfectedDamage": "infected",       # claw slashes over a health bar
    "QuickTransfer": "transfer",        # cursor sweeping items from a pack into a crate
    "CrashSafety": "crash",             # a heli tipped over on landing, a car into a wall, hull bar held at 10%
    "AutoRun": "autorun",               # the running-man sign with three speed lines behind it
    "HouseRaidRules": "houseraid",      # a rebuilt brick house, a PvE shield over it stopping a bullet
    "SatchelSoundFix": "satchel",       # a satchel charge after the bang, its sound waves crossed out
    "HouseCodeLock": "codelock",        # a plain house door with a square keypad lock on it
    "BarrelStorage": "barrelrack",      # a rack, three shelves of two barrels in the vanilla colours
}

# Mods based on someone else's work: shown in a Credits section, and the licence line says so
CREDITS = {
    "BarrelStorage": (
        "Based on [b]INVICTUZ_Barrel_Storage[/b] by [b]Xantarn[/b]: the rack, the kit, their models and textures are Xantarn's work. Xantarn gave permission to do anything with it. Original: [url=https://steamcommunity.com/sharedfiles/filedetails/?id=3406014628]Workshop 3406014628[/url].",
        "[b]Licence:[/b] the DevZ fixes are MIT. The models and textures are Xantarn's, used with permission; please credit Xantarn if you reuse them.",
    ),
}
# A bundle's picture is one small tile per mod in it, each with that mod's pictogram and colour
# (filled in from PACKS below).


# Bundles: one Workshop item holding several client mods (built by build_pack.ps1)
# folder: (title on the image, accent colour, one-line summary, {mod in it: one line on what it does})
PACKS = {
    "Essentials": (
        "DevZ\nEssentials", (120, 200, 140),
        "Seven DevZ mods in one download: bigger inventories, bigger ammo stacks, longer-lasting guns, kits that combine, repairs back to Pristine, quick looting with one key, and auto run.",
        {
            "InventorySizes": "more room in clothing, backpacks, vests and pouches",
            "AmmoStacks": "loose ammo stacks twice as big",
            "WeaponDurability": "weapons and suppressors last 10x as long (the plastic bottle suppressor 3.3x)",
            "CombineNoSplit": "combine part-used bandages, repair kits and filters, but never split them",
            "RepairToPristine": "repair kits repair items all the way to Pristine instead of stopping at Worn",
            "QuickTransfer": "hold Left Alt and sweep the mouse over items to move them between you and the nearest open container",
            "AutoRun": "press Z and your character keeps moving forward at the speed you had (walk, jog or sprint); W, A, S, D or Z stops it",
        },
    ),
}


for folder, (_, _, _, mods) in PACKS.items():
    PICTOGRAMS[folder] = [(PICTOGRAMS[m], MODS[m][1]) for m in mods]


def preview(folder, title, accent, path):
    if folder in SERVER_ONLY:
        footer = "server mod  ·  by DevilinJake"
    elif folder in PACKS:
        footer = f"{len(PACKS[folder][3])} mods in one  ·  by DevilinJake"
    else:
        footer = "by DevilinJake"
    # mods added after the first bundle go last, so the older cards keep their texture
    order = list(MODS)[:8] + list(PACKS) + list(MODS)[8:]
    seed = order.index(folder) * 10        # fixes the background texture per mod
    card(title.split("\n"), accent, PICTOGRAMS[folder], seed, path, footer=footer)


def description(folder, summary, what, defaults):
    url = f"{GITHUB}/tree/main/{folder}"
    bullets = lambda items: "[list]\n" + "".join(f"[*] {i}\n" for i in items) + "[/list]"
    server = folder in SERVER_ONLY
    scans = SCANS[folder]

    if server:
        setup = bullets(SETUP_EXTRA.get(folder, []) + [
            "Copy the mod folder into your server folder and add it to [b]-servermod=[/b] (not -mod=).",
            "Players don't download anything, and it needs no key.",
        ])
    else:
        setup = bullets(SETUP_EXTRA.get(folder, []) + [
            "Add it to [b]-mod=[/b] (players need it too; they download it automatically when they join).",
            "Copy [b]keys\\DevZ.bikey[/b] from the mod folder into your server's [b]keys[/b] folder.",
        ])

    if scans:
        covers = "This Workshop build covers [b]vanilla items[/b], so it works on any server and needs no other mod. Running modded items? See below.\n"
        own_copy = "upload your copy to the Workshop (Unlisted is fine) or put it in your server pack, and use it instead of this one" if not server \
            else "load your copy with -servermod instead of this one (players still download nothing)"
        others = f"""The mod comes with a [b]scanner[/b] that reads your server and every mod you run, and lists {scans} in a simple table, next to the vanilla ones. Pick your own values for any of them (or keep the defaults), build it, and you have a copy made for your exact mod list.
[list]
[*] The settings are built into the mod, so this Workshop download only holds the finished mod. The tables, scanner and build script are on [url={GITHUB}]GitHub[/url].
[*] No programming: download the repo, run one command to scan, edit the table in Excel or Google Sheets, run one command to build. You need Python and DayZ Tools (both free).
[*] Step-by-step guide: [url={GITHUB}/blob/main/CUSTOMISING.md]CUSTOMISING.md[/url]
[*] Then {own_copy}.
[/list]"""
    else:
        covers = ""
        others = NO_SCAN[folder]

    licence = "[b]Licence: MIT.[/b] Use it, change it and repack it, including in your own server pack. A credit is appreciated but not required."
    credits = ""
    if folder in CREDITS:
        credits = f"\n[h2]Credits[/h2]\n{CREDITS[folder][0]}\n"
        licence = CREDITS[folder][1]

    return f"""{summary}

{bullets(what)}

[h2]Default settings[/h2]
{bullets(defaults)}
{covers}
[h2]Server setup[/h2]
{setup}

[h2]{"Your own settings and modded items" if scans else "Works with your other mods"}[/h2]
{others}

[h2]Links[/h2]
[list]
[*] Details and source: [url={url}]GitHub[/url]
[*] More DevZ mods: [url={GITHUB}]DevZ-Mods[/url]
[/list]
{credits}
{licence}
"""


def pack_description(folder, summary, mods):
    bullets = lambda items: "[list]\n" + "".join(f"[*] {i}\n" for i in items) + "[/list]"
    name = lambda m: MODS[m][0].replace("\n", " ")
    included = [f"[b]{name(m)}[/b]: {line}." for m, line in mods.items()]
    defaults = [f"[b]{name(m)}:[/b] {' '.join(MODS[m][4])}" for m in mods]
    pages = [f"[url={GITHUB}/tree/main/{m}]{name(m)}[/url]" for m in mods]
    return f"""{summary}

[h2]What's inside[/h2]
{bullets(included)}
Each one is also on the Workshop on its own. [b]Use either this bundle or the single mods, never both[/b]: they're the same files, so loading both loads every change twice.

[h2]Default settings[/h2]
{bullets(defaults)}
This Workshop build covers [b]vanilla items[/b], so it works on any server and needs no other mod.

[h2]Server setup[/h2]
[list]
[*] Add it to [b]-mod=[/b] (players need it too; they download it automatically when they join).
[*] Copy [b]keys\\DevZ.bikey[/b] from the mod folder into your server's [b]keys[/b] folder.
[*] Ammo Stacks: run [b]fix_loot.py[/b] from the GitHub repo once on your types.xml so the ammo found in the world stays the same size.
[/list]

[h2]Your own settings and modded items[/h2]
The mods with item tables come with a [b]scanner[/b] that reads your server and every mod you run (clothing, weapons, kits...) and lists their items in a simple table, next to the vanilla ones. Change any values, build, and you have copies made for your exact mod list.
[list]
[*] The settings are built into the mods, so this download only holds the finished mods. The tables, scanners and build scripts are on [url={GITHUB}]GitHub[/url].
[*] Step-by-step guide, no programming needed: [url={GITHUB}/blob/main/CUSTOMISING.md]CUSTOMISING.md[/url]
[*] Want only some of them, or your own settings for one? Load the single mods (or your own copies) instead of this bundle.
[/list]

[h2]Links[/h2]
[list]
[*] Each mod's details: {", ".join(pages)}
[*] More DevZ mods: [url={GITHUB}]DevZ-Mods[/url]
[/list]

[b]Licence: MIT.[/b] Use it, change it and repack it, including in your own server pack. A credit is appreciated but not required.
"""


for folder, (title, accent, summary, what, defaults) in MODS.items():
    out = REPO / folder / "workshop"
    out.mkdir(exist_ok=True)
    preview(folder, title, accent, out / "preview.png")
    (out / "description.txt").write_text(description(folder, summary, what, defaults), encoding="utf-8")
    print(f"{folder}: {out}")

for folder, (title, accent, summary, mods) in PACKS.items():
    out = REPO / folder / "workshop"
    out.mkdir(parents=True, exist_ok=True)
    preview(folder, title, accent, out / "preview.png")
    (out / "description.txt").write_text(pack_description(folder, summary, mods), encoding="utf-8")
    print(f"{folder}: {out}")
