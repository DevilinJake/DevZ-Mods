"""Builds items.csv: every item with uses, a count or a percentage (bandages, repair kits,
filters, rags...) so admins can pick which ones combine but never split.

Reads the configs straight out of the game's and mods' PBO files, so it also lists items from
any mod you point it at. Re-running keeps the Combine choices you already made. New items get
Combine = Yes only if they're in the default list below.

Usage:
    python scan_items.py <DayZ or DayZServer folder> [mod folder ...] [-o items.csv]

Example:
    python scan_items.py "C:/Program Files (x86)/Steam/steamapps/common/DayZServer" ^
        "C:/.../DayZServer/@SomeMod"
"""
import argparse, csv, os, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tools'))
from dayz_config import ancestors, load_classes, load_stringtables, resolve
from mod_scan import find_pbos, source_label

COLUMNS = ['Combine', 'Class', 'Example', 'MaxQuantity', 'ShownAs', 'SplitsNow', 'Source']

# Ticked the first time an item is listed
DEFAULTS = {'bandagedressing', 'ducttape', 'electronicrepairkit', 'epoxyputty', 'leathersewingkit',
            'sewingkit', 'tirerepairkit', 'weaponcleaningkit', 'whetstone',
            'gasmask_filter', 'gasmask_filter_improvised'}

# Ammo, food, drinks, clothing, vehicles and cooking gear use quantity for something else
SKIP_BASES = {'magazine_base', 'ammunition_base', 'edible_base', 'bottle_base', 'clothing_base',
              'transport', 'carscript', 'fireplacebase', 'pot'}

# Vanilla already lets these combine (in script), so they don't need the mod
ALREADY_COMBINE = {'matchbox', 'plantmaterial', 'disinfectantalcohol', 'iodinetincture'}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), 'items.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    classes = load_classes(pbos)
    strings = load_stringtables(pbos)

    def prop(cls, name, default=None):
        return resolve(classes, cls, name, default)[0]

    def display_name(cls):
        name = prop(cls, 'displayName', '')
        if isinstance(name, str) and name.startswith('$'):
            name = strings.get(name[1:].lower(), name)
        return name or classes[cls]['name']

    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    rows = []
    for cls in classes:
        if prop(cls, 'scope', 0) != 2 or cls in ALREADY_COMBINE:
            continue
        try:
            qmax = float(prop(cls, 'varQuantityMax', 0) or 0)
        except (TypeError, ValueError):
            continue
        if qmax <= 0 or prop(cls, 'liquidContainerType', 0):
            continue
        if SKIP_BASES & set(ancestors(classes, cls)):
            continue
        prev = old.get(cls)
        rows.append({
            'Combine': prev['Combine'] if prev else ('Yes' if cls in DEFAULTS else ''),
            'Class': classes[cls]['name'],
            'Example': display_name(cls),
            'MaxQuantity': int(qmax) if qmax.is_integer() else qmax,
            'ShownAs': 'bar' if prop(cls, 'quantityBar', 0) else 'count',
            # canBeSplit = 1 (rags, nails, logs): the game already combines and splits these
            'SplitsNow': 'Yes' if prop(cls, 'canBeSplit', 0) else '',
            'Source': source_label(classes[cls]['source']),
        })

    # Keep hand-added rows for items this scan didn't see (e.g. a mod left out this time)
    seen = {r['Class'].lower() for r in rows}
    rows += [r for c, r in old.items() if c not in seen]

    rows.sort(key=lambda r: (r['Combine'].strip().lower() != 'yes', r['Source'] != 'Vanilla', r['Source'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS, extrasaction='ignore')
        w.writeheader()
        w.writerows(rows)
    ticked = sum(r['Combine'].strip().lower() == 'yes' for r in rows)
    print(f'{len(rows)} items written to {args.output} ({ticked} set to combine)')


if __name__ == '__main__':
    main()
