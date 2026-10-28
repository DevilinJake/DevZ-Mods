"""Builds kits.csv: every repair kit in the game and your mods, so admins can pick which ones
repair all the way to Pristine.

A repair kit is any item with a repairKitType in its config (sewing kits, gun cleaning kit,
duct tape, blowtorch...). Reads the configs straight out of the game's and mods' PBO files.
Re-running keeps the Pristine choices you already made. New kits get Pristine = Yes.

Usage:
    python scan_kits.py <DayZ or DayZServer folder> [mod folder ...] [-o kits.csv]

Example:
    python scan_kits.py "C:/Program Files (x86)/Steam/steamapps/common/DayZServer" ^
        "C:/.../DayZServer/@SomeMod"
"""
import argparse, collections, csv, os, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tools'))
from dayz_config import load_classes, load_stringtables, resolve
from mod_scan import find_pbos, source_label

COLUMNS = ['Pristine', 'Class', 'Example', 'KitType', 'Repairs', 'Source']


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), 'kits.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    strings = load_stringtables(pbos)

    kits = {}
    repairs = collections.Counter()  # kit type -> how many items it can repair
    for root in ('CfgVehicles', 'CfgWeapons'):
        classes = load_classes(pbos, root)

        def prop(cls, name, default=None):
            return resolve(classes, cls, name, default)[0]

        for cls, entry in classes.items():
            if prop(cls, 'scope', 0) != 2:
                continue
            for t in prop(cls, 'repairableWithKits', []) or []:
                repairs[t] += 1
            kit_type = prop(cls, 'repairKitType', 0)
            if not isinstance(kit_type, int) or kit_type <= 0:
                continue
            name = prop(cls, 'displayName', '')
            if isinstance(name, str) and name.startswith('$'):
                name = strings.get(name[1:].lower(), name)
            kits[cls] = {'Class': entry['name'], 'Example': name or entry['name'], 'KitType': kit_type,
                         'Source': source_label(entry['source'])}

    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    rows = []
    for cls, kit in kits.items():
        prev = old.get(cls)
        rows.append({'Pristine': prev['Pristine'] if prev else 'Yes', **kit, 'Repairs': repairs[kit['KitType']]})

    # Keep hand-added rows for kits this scan didn't see (e.g. a mod left out this time)
    rows += [r for c, r in old.items() if c not in kits]

    rows.sort(key=lambda r: (r['Source'] != 'Vanilla', r['Source'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS, extrasaction='ignore')
        w.writeheader()
        w.writerows(rows)
    ticked = sum(r['Pristine'].strip().lower() == 'yes' for r in rows)
    print(f'{len(rows)} kits written to {args.output} ({ticked} repair to Pristine)')


if __name__ == '__main__':
    main()
