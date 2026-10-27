"""Builds sizes.csv: every clothing item, backpack and clothing attachment (pouches) with a cargo grid,
plus vests, belts and holsters that have none, so admins can give them one.

Reads the configs straight out of the game's and mods' PBO files, so it also picks up items
from any clothing mod you point it at. Re-running keeps the NewWidth/NewHeight you already
filled in.

Usage:
    python scan_sizes.py <DayZ or DayZServer folder> [mod folder ...] [-o sizes.csv]

Example:
    python scan_sizes.py "C:/Program Files (x86)/Steam/steamapps/common/DayZServer" ^
        "C:/.../DayZServer/@ExpansionBundle" "C:/.../DayZServer/@ExpansionLicensed"
"""
import argparse, csv, os, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tools'))
from dayz_config import (ancestors, load_classes, load_stringtables, read_configs,
                         resolve)
from mod_scan import find_pbos, source_label

COLUMNS = ['Category', 'Class', 'Parent', 'Example', 'Items', 'Source', 'RequiredAddon',
           'CurrentWidth', 'CurrentHeight', 'CurrentSlots', 'NewWidth', 'NewHeight']

# Inventory slot -> category shown in the table, in the order rows are sorted
CATEGORIES = [('Back', 'Backpacks'), ('Body', 'Tops'), ('Vest', 'Vests'), ('Legs', 'Pants'),
              ('Hips', 'Belts'), ('VestPouch', 'Pouches'), ('VestBackpack', 'Pouches'),
              ('VestHolster', 'Holsters'), ('Headgear', 'Headgear'), ('Mask', 'Masks'), ('Feet', 'Shoes'),
              ('Gloves', 'Gloves'), ('Eyewear', 'Eyewear'), ('Armband', 'Armbands')]

# Wearables in these slots get a row even with no grid, so admins can give them one
NO_GRID_SLOTS = {'Vest', 'Hips', 'VestHolster', 'VestPouch', 'VestBackpack'}
GENERIC_BASES = {'clothing', 'clothing_base', 'container_base', 'inventory_base', 'static', 'all'}


def defining_addons(pbos):
    """For each class (lower-case), the CfgPatches name and source of the PBO that first defines it."""
    owner = {}
    for pbo in pbos:
        for cfg in read_configs(pbo):
            patches = [k for k in next((v for k, v in cfg.items() if k.lower() == 'cfgpatches'), {})
                       if not k.startswith('__')]
            section = next((v for k, v in cfg.items() if k.lower() == 'cfgvehicles'), {})
            for name, body in section.items():
                if isinstance(body, dict) and body.get('__parent') and name.lower() not in owner and patches:
                    owner[name.lower()] = (patches[0], source_label(pbo))
    return owner


def category(slots):
    if isinstance(slots, str):
        slots = [slots]
    for slot, label in CATEGORIES:
        if slot in (slots or []):
            return label
    return 'Other'


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), 'sizes.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    classes = load_classes(pbos)
    owner = defining_addons(pbos)
    strings = load_stringtables(pbos)

    def display_name(cls):
        name, _ = resolve(classes, cls, 'displayName', '')
        if isinstance(name, str) and name.startswith('$'):
            name = strings.get(name[1:].lower(), name)
        return name or classes[cls]['name']

    def slots_of(cls):
        slots, _ = resolve(classes, cls, 'inventorySlot')
        return [slots] if isinstance(slots, str) else (slots or [])

    # Slots that clothing accepts attachments in (vest pouches, holsters, belt slots...)
    clothing = {c for c in classes if 'clothing_base' in ancestors(classes, c)}
    attach_slots = {str(s).lower() for c in clothing for s in (resolve(classes, c, 'attachments', [])[0] or [])}

    # Group every spawnable clothing item or clothing attachment under the class its grid comes from
    groups = {}
    for cls in classes:
        attaches = any(str(s).lower() in attach_slots for s in slots_of(cls))
        if cls not in clothing and not attaches:
            continue
        scope, _ = resolve(classes, cls, 'scope', 0)
        size, defined_by = resolve(classes, cls, 'itemsCargoSize')
        if scope != 2 or not defined_by or not size or len(size) < 2 or defined_by not in owner:
            continue
        groups.setdefault(defined_by, []).append(cls)

    # Wearables with no grid of their own (plate carrier, chest holster, belts), so admins can add one
    no_grid = {}
    for cls in clothing:
        size, _ = resolve(classes, cls, 'itemsCargoSize')
        if resolve(classes, cls, 'scope', 0)[0] != 2 or size or not NO_GRID_SLOTS & set(slots_of(cls)):
            continue
        # Group colour versions under their top base class below the generic clothing classes
        chain = ancestors(classes, cls)
        key = next((c for c in reversed(chain) if c not in GENERIC_BASES), cls)
        if key in owner:
            no_grid.setdefault(key, []).append(cls)

    # Keep values already filled in
    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    def row(cls, items, width, height):
        prev = old.get(cls, {})
        parent = classes[cls]['parent']
        return {
            'Category': category(slots_of(cls)),
            'Class': classes[cls]['name'],
            'Parent': classes[parent]['name'] if parent in classes else parent,
            'Example': display_name(min(items, key=lambda c: (classes[c]['source'] != classes[cls]['source'], c))),
            'Items': len(items),
            'Source': owner[cls][1],
            'RequiredAddon': owner[cls][0],
            'CurrentWidth': width, 'CurrentHeight': height, 'CurrentSlots': (width or 0) * (height or 0),
            'NewWidth': prev.get('NewWidth', ''), 'NewHeight': prev.get('NewHeight', ''),
        }

    rows = []
    for cls, items in groups.items():
        width, height = classes[cls]['props']['itemsCargoSize'][:2]
        rows.append(row(cls, items, int(width), int(height)))
    for cls, items in no_grid.items():
        rows.append(row(cls, items, '', ''))

    order = list(dict.fromkeys(label for _, label in CATEGORIES)) + ['Other']
    rows.sort(key=lambda r: (order.index(r['Category']), r['Source'] != 'Vanilla', -r['CurrentSlots'], r['Class']))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS)
        w.writeheader()
        w.writerows(rows)
    print(f'{len(rows)} cargo grids ({sum(r["Items"] for r in rows)} items) written to {args.output}')


if __name__ == '__main__':
    main()
