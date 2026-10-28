"""Builds recipes.csv: every right-click crafting recipe in the game and the mods you point it
at, so admins can pick which ones to remove.

Reads the recipe scripts straight out of the PBO files. Re-running keeps the Remove choices you
already made. New recipes get Remove = Yes only if they're in the default list below.

Usage:
    python scan_recipes.py <DayZ or DayZServer folder> [mod folder ...] [-o recipes.csv]

Example:
    python scan_recipes.py "C:/Program Files (x86)/Steam/steamapps/common/DayZServer" ^
        "C:/.../DayZServer/@SomeMod"
"""
import argparse, csv, os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tools'))
from dayz_config import list_pbo_files, load_stringtables, read_pbo_file
from mod_scan import find_pbos, source_label

COLUMNS = ['Remove', 'Class', 'Name', 'Item1', 'Item2', 'Results', 'Source']

# Ticked the first time a recipe is listed: iM7s Drug Running's coca paste shortcuts, which
# make more cocaine than the Chemical Barrel and skip its Sulfuric Acid
DEFAULTS = {'im7sdr_craftcocapaste', 'im7sdr_craftcocapaste_dried'}

CLASS_RE = re.compile(r'^\s*class\s+(\w+)\s+extends\s+(\w+)', re.M)


def short_list(names, keep=3):
    """'A / B / C +12 more' so long tool lists stay readable."""
    names = list(dict.fromkeys(names))
    text = ' / '.join(names[:keep])
    return text + (f' +{len(names) - keep} more' if len(names) > keep else '')


def recipes_in(text):
    """(class, name, item1 list, item2 list, results) for each recipe class in a script."""
    starts = [(m.start(), m.group(1)) for m in CLASS_RE.finditer(text)]
    for i, (pos, cls) in enumerate(starts):
        body = text[pos:starts[i + 1][0] if i + 1 < len(starts) else len(text)]
        ingredients = re.findall(r'InsertIngredient\s*\(\s*(\d)\s*,\s*"(\w+)"', body)
        name = re.search(r'm_Name\s*=\s*"([^"]*)"', body)
        if not ingredients or not name:
            continue
        yield (cls, name.group(1),
               [n for slot, n in ingredients if slot == '0'],
               [n for slot, n in ingredients if slot == '1'],
               re.findall(r'AddResult\s*\(\s*"(\w+)"', body))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), 'recipes.csv'))
    args = ap.parse_args()

    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    pbos = find_pbos(args.folders)
    strings = load_stringtables(pbos)
    found = {}
    for pbo in pbos:
        for path in list_pbo_files(pbo):
            if not path.lower().endswith('.c'):
                continue
            data = read_pbo_file(pbo, path)
            if not data:
                continue
            for cls, name, item1, item2, results in recipes_in(data.decode('utf-8', 'replace')):
                if name[:1] in '#$':  # vanilla names are stringtable keys
                    name = strings.get(name[1:].lower(), name)
                prev = old.get(cls.lower())
                found[cls.lower()] = {
                    'Remove': prev['Remove'] if prev else ('Yes' if cls.lower() in DEFAULTS else ''),
                    'Class': cls,
                    'Name': name,
                    'Item1': short_list(item1),
                    'Item2': short_list(item2),
                    'Results': short_list(results),
                    'Source': source_label(pbo),
                }

    # Keep hand-added rows for recipes this scan didn't see (e.g. a mod left out this time)
    rows = list(found.values()) + [r for c, r in old.items() if c not in found]
    rows.sort(key=lambda r: (r['Remove'].strip().lower() != 'yes', r['Source'] != 'Vanilla', r['Source'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS, extrasaction='ignore')
        w.writeheader()
        w.writerows(rows)
    ticked = sum(r['Remove'].strip().lower() == 'yes' for r in rows)
    print(f'{len(rows)} recipes written to {args.output} ({ticked} set to remove)')


if __name__ == '__main__':
    main()
