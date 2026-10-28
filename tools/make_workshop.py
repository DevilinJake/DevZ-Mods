"""Writes each client mod's Workshop preview image and description into <Mod>\\workshop\\.

  python tools\\make_workshop.py

preview.png goes in Publisher's preview image box, description.txt (Steam BBCode) in its
description box. Edit the MODS table below, then re-run.
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

REPO = Path(__file__).resolve().parent.parent
GITHUB = "https://github.com/DevilinJake/DevZ-Mods"
FONTS = Path("C:/Windows/Fonts")

# folder: (title on the image, accent colour, one-line summary, what it does, defaults)
MODS = {
    "InventorySizes": (
        "Inventory\nSizes", (86, 156, 214),
        "Bigger cargo for clothing, backpacks, vests and pouches, with sizes you choose.",
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
        "Bigger loose-ammo stacks: one multiplier for everything, per-ammo overrides.",
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
        "Weapons and suppressors wear out slower: one multiplier each, per-weapon overrides.",
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
        "Switch off right-click crafting recipes from vanilla or any mod, without editing that mod.",
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
}


def font(name, size):
    return ImageFont.truetype(str(FONTS / name), size)


def preview(title, accent, path):
    size = 512
    img = Image.new("RGB", (size, size), (24, 26, 30))
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, size, 14], fill=accent)
    d.rectangle([0, size - 14, size, size], fill=accent)
    d.text((40, 52), "DevZ", font=font("bahnschrift.ttf", 64), fill=accent)
    big = 76 if title.count("\n") < 2 else 62  # three lines need a smaller size to clear the footer
    d.multiline_text((40, 170 if big == 76 else 150), title, font=font("segoeuib.ttf", big), fill=(236, 236, 236), spacing=6)
    d.text((40, 440), "DayZ mod  ·  MIT  ·  by DevilinJake", font=font("segoeui.ttf", 24), fill=(150, 150, 150))
    img.save(path, optimize=True)


def description(folder, summary, what, defaults):
    url = f"{GITHUB}/tree/main/{folder}"
    bullets = lambda items: "[list]\n" + "".join(f"[*] {i}\n" for i in items) + "[/list]"
    return f"""{summary}

{bullets(what)}

[h2]Default settings[/h2]
{bullets(defaults)}
This build changes [b]vanilla items only[/b], so it works on any server and needs no other mod.

[h2]Server setup[/h2]
[list]
[*] Add it to [b]-mod=[/b] (players need it too; they download it automatically when they join).
[*] Copy [b]keys\\DevZ.bikey[/b] from the mod folder into your server's [b]keys[/b] folder.
[/list]

[h2]Want different settings?[/h2]
The settings are built into the mod, so changing them means building your own copy: download the repo, run the scanner on your server (it picks up items from your other mods too), change a table and build it. Step-by-step guide, no programming needed: [url={GITHUB}/blob/main/CUSTOMISING.md]CUSTOMISING.md[/url]. Then upload your copy (Unlisted is fine) or put it in your server pack.

[h2]Links[/h2]
[list]
[*] Details and source: [url={url}]GitHub[/url]
[*] More DevZ mods: [url={GITHUB}]DevZ-Mods[/url]
[/list]

[b]Licence: MIT.[/b] Use it, change it and repack it, including in your own server pack. A credit is appreciated but not required.
"""


for folder, (title, accent, summary, what, defaults) in MODS.items():
    out = REPO / folder / "workshop"
    out.mkdir(exist_ok=True)
    preview(title, accent, out / "preview.png")
    (out / "description.txt").write_text(description(folder, summary, what, defaults), encoding="utf-8")
    print(f"{folder}: {out}")
