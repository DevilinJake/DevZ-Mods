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
            "Suppressors and other muzzle attachments last 10x as long.",
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
}

# Server-side mods: loaded with -servermod, players never download them
SERVER_ONLY = {"AutoStack", "InfectedDamage"}


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
    "Essentials": "essentials",         # the bundle's five pictograms together
}


# Bundles: one Workshop item holding several client mods (built by build_pack.ps1)
# folder: (title on the image, accent colour, one-line summary, {mod in it: one line on what it does})
PACKS = {
    "Essentials": (
        "DevZ\nEssentials", (120, 200, 140),
        "Five DevZ mods in one download: bigger inventories, bigger ammo stacks, longer-lasting guns, kits that combine, and repairs back to Pristine.",
        {
            "InventorySizes": "more room in clothing, backpacks, vests and pouches",
            "AmmoStacks": "loose ammo stacks twice as big",
            "WeaponDurability": "weapons and suppressors last 10x as long",
            "CombineNoSplit": "combine part-used bandages, repair kits and filters, but never split them",
            "RepairToPristine": "repair kits repair items all the way to Pristine instead of stopping at Worn",
        },
    ),
}


def preview(folder, title, accent, path):
    footer = "server mod  ·  by DevilinJake" if folder in SERVER_ONLY else "by DevilinJake"
    order = list(MODS) + list(PACKS)
    seed = order.index(folder) * 10        # fixes the background texture per mod
    card(title.split("\n"), accent, PICTOGRAMS[folder], seed, path, footer=footer)


def description(folder, summary, what, defaults):
    url = f"{GITHUB}/tree/main/{folder}"
    bullets = lambda items: "[list]\n" + "".join(f"[*] {i}\n" for i in items) + "[/list]"
    server = folder in SERVER_ONLY
    scans = SCANS[folder]

    if server:
        setup = """[list]
[*] Copy the mod folder into your server folder and add it to [b]-servermod=[/b] (not -mod=).
[*] Players don't download anything, and it needs no key.
[/list]"""
    else:
        setup = """[list]
[*] Add it to [b]-mod=[/b] (players need it too; they download it automatically when they join).
[*] Copy [b]keys\\DevZ.bikey[/b] from the mod folder into your server's [b]keys[/b] folder.
[/list]"""

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
        others = "Works with any ammo, vanilla or modded, and any stack size, including [b]DevZ Ammo Stacks[/b]. Nothing to set up."

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

[b]Licence: MIT.[/b] Use it, change it and repack it, including in your own server pack. A credit is appreciated but not required.
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
Every mod in the bundle comes with a [b]scanner[/b] that reads your server and every mod you run (clothing, weapons, kits...) and lists their items in a simple table, next to the vanilla ones. Change any values, build, and you have copies made for your exact mod list.
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
