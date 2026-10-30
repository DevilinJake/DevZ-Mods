r"""Draws only the DevZ Essentials bundle card (Essentials\workshop\preview.png), one tile per mod
listed in Essentials\pack.txt, without rewriting any other card or description.

make_workshop.py draws the same card from its PACKS table (along with every other card and
description); this script reads the mod list from pack.txt instead, and each mod's accent colour
from make_workshop.py's MODS table, so the tiles match the single mods' cards.

    python tools/essentials_card.py
"""
import ast
from pathlib import Path

from workshop_preview import card

HERE = Path(__file__).resolve().parent
REPO = HERE.parent

# make_workshop.py writes every card and description when run, so read its tables without running it
tables = {}
for node in ast.parse((HERE / "make_workshop.py").read_text(encoding="utf-8")).body:
    if isinstance(node, ast.Assign) and getattr(node.targets[0], "id", "") in ("MODS", "PICTOGRAMS"):
        tables[node.targets[0].id] = ast.literal_eval(node.value)
MODS, PICTOGRAMS = tables["MODS"], tables["PICTOGRAMS"]

lines = (REPO / "Essentials" / "pack.txt").read_text(encoding="utf-8").splitlines()
mods = [m.strip() for m in lines if m.strip() and not m.startswith("#")]
tiles = [(PICTOGRAMS[m], MODS[m][1]) for m in mods]

# same title, colour and background texture (seed) as make_workshop.py gives the bundle
seed = 8 * 10
out = REPO / "Essentials" / "workshop" / "preview.png"
card(["DevZ", "Essentials"], (120, 200, 140), tiles, seed, out,
     footer=f"{len(mods)} mods in one  ·  by DevilinJake")
print(f"{out}: {', '.join(mods)}")
