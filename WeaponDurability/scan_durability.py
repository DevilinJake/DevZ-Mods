"""Builds durability.csv: every weapon, suppressor and muzzle attachment and how tough it is.

DayZ wears a weapon a little on every shot: the ammo's damageBarrel divided by the weapon's
barrelArmor (suppressors and other muzzle attachments have their own barrelArmor and wear the
same way). This lists every class that sets barrelArmor, so its durability can be changed.

Re-running keeps the Multiplier values you already set.

Usage:
    python scan_durability.py <DayZ or DayZServer folder> [mod folder ...] [-o durability.csv]
"""
import argparse, csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'tools'))
from dayz_config import ancestors, load_classes, load_stringtables, read_configs, resolve
from mod_scan import find_pbos, source_label

COLUMNS = ['Type', 'Class', 'Parent', 'Section', 'Example', 'Items', 'Source', 'RequiredAddon',
           'CurrentArmor', 'Multiplier']


def defining_addons(pbos, section_name):
    owner = {}
    for pbo in pbos:
        for cfg in read_configs(pbo):
            patches = [k for k in next((v for k, v in cfg.items() if k.lower() == 'cfgpatches'), {})
                       if not k.startswith('__')]
            section = next((v for k, v in cfg.items() if k.lower() == section_name.lower()), {})
            for name, body in section.items():
                if isinstance(body, dict) and body.get('__parent') and name.lower() not in owner and patches:
                    owner[name.lower()] = (patches[0], source_label(pbo))
    return owner


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(HERE, 'durability.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    strings = load_stringtables(pbos)
    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    rows = []
    # Weapons live in CfgWeapons; suppressors and other muzzle attachments in CfgVehicles
    for section, wanted in (('CfgWeapons', None), ('CfgVehicles', 'itemsuppressor')):
        classes = load_classes(pbos, section)
        owner = defining_addons(pbos, section)

        def display_name(cls):
            name, _ = resolve(classes, cls, 'displayName', '')
            if isinstance(name, str) and name.startswith('$'):
                name = strings.get(name[1:].lower(), name)
            return name if name and name.upper() != 'TODO' else classes[cls]['name']

        groups = {}
        for cls in classes:
            if wanted and wanted not in ancestors(classes, cls):
                continue
            if resolve(classes, cls, 'scope', 0)[0] != 2:
                continue
            armor, defined_by = resolve(classes, cls, 'barrelArmor')
            if not armor or defined_by not in owner:
                continue
            groups.setdefault(defined_by, []).append(cls)

        for cls, items in groups.items():
            parent = classes[cls]['parent']
            name = classes[cls]['name']
            if section == 'CfgWeapons':
                kind = 'Weapon'
            else:
                kind = 'Suppressor' if 'suppressor' in name.lower() else 'Muzzle'
            rows.append({
                'Type': kind,
                'Class': name,
                'Parent': classes[parent]['name'] if parent in classes else parent,
                'Section': section,
                'Example': display_name(min(items, key=lambda c: (classes[c]['source'] != classes[cls]['source'], c))),
                'Items': len(items),
                'Source': owner[cls][1],
                'RequiredAddon': owner[cls][0],
                'CurrentArmor': round(float(classes[cls]['props']['barrelArmor']), 4),
                'Multiplier': old.get(cls, {}).get('Multiplier', ''),
            })

    order = ['Weapon', 'Suppressor', 'Muzzle']
    rows.sort(key=lambda r: (order.index(r['Type']), r['Source'] != 'Vanilla', r['Source'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS)
        w.writeheader()
        w.writerows(rows)
    print(f'{len(rows)} rows ({sum(r["Items"] for r in rows)} items) written to {args.output}')


if __name__ == '__main__':
    main()
