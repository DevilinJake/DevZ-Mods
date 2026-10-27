"""Builds ammo.csv: every loose-ammo pile and how many rounds fit in one stack.

Reads the configs straight out of the game's and mods' PBO files, so it also picks up ammo
from any weapon mod you point it at. Re-running keeps the NewStack values you already set.

Usage:
    python scan_ammo.py <DayZ or DayZServer folder> [mod folder ...] [-o ammo.csv]
"""
import argparse, csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'tools'))
from dayz_config import ancestors, load_classes, load_stringtables, read_configs, resolve
from mod_scan import find_pbos, source_label

COLUMNS = ['Class', 'Parent', 'Example', 'Source', 'RequiredAddon', 'CurrentStack', 'NewStack']


def defining_addons(pbos):
    """For each CfgMagazines class (lower-case), the CfgPatches name and source of the PBO defining it."""
    owner = {}
    for pbo in pbos:
        for cfg in read_configs(pbo):
            patches = [k for k in next((v for k, v in cfg.items() if k.lower() == 'cfgpatches'), {})
                       if not k.startswith('__')]
            section = next((v for k, v in cfg.items() if k.lower() == 'cfgmagazines'), {})
            for name, body in section.items():
                if isinstance(body, dict) and body.get('__parent') and name.lower() not in owner and patches:
                    owner[name.lower()] = (patches[0], source_label(pbo))
    return owner


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(HERE, 'ammo.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    classes = load_classes(pbos, 'CfgMagazines')
    owner = defining_addons(pbos)
    strings = load_stringtables(pbos)

    def display_name(cls):
        name, _ = resolve(classes, cls, 'displayName', '')
        if isinstance(name, str) and name.startswith('$'):
            name = strings.get(name[1:].lower(), name)
        # A few vanilla items still have placeholder names like 'TODO'
        return name if name and name.upper() != 'TODO' else classes[cls]['name']

    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    rows = []
    for cls in classes:
        if 'ammunition_base' not in ancestors(classes, cls) or cls not in owner:
            continue
        if resolve(classes, cls, 'scope', 0)[0] != 2:
            continue
        count, _ = resolve(classes, cls, 'count')
        if not count:
            continue
        parent = classes[cls]['parent']
        rows.append({
            'Class': classes[cls]['name'],
            'Parent': classes[parent]['name'] if parent in classes else parent,
            'Example': display_name(cls),
            'Source': owner[cls][1],
            'RequiredAddon': owner[cls][0],
            'CurrentStack': int(count),
            'NewStack': old.get(cls, {}).get('NewStack', ''),
        })

    rows.sort(key=lambda r: (r['Source'] != 'Vanilla', r['Source'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS)
        w.writeheader()
        w.writerows(rows)
    print(f'{len(rows)} ammo types written to {args.output}')


if __name__ == '__main__':
    main()
