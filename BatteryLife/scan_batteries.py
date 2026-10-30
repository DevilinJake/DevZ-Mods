"""Builds batteries.csv: every battery and gas canister and how much energy it holds.

A battery (or gas canister) holds energyStorageMax units of energy, and a device uses its own
energyUsagePerSecond while it's on. So a bigger energyStorageMax makes every device that runs on
that battery last longer. This lists every item that stores energy for other items to use.

Re-running keeps the Multiplier values you already set.

Usage:
    python scan_batteries.py <DayZ or DayZServer folder> [mod folder ...] [-o batteries.csv]
"""
import argparse, csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'tools'))
from dayz_config import ancestors, load_classes, load_stringtables, read_configs, resolve
from mod_scan import find_pbos, source_label

COLUMNS = ['Type', 'Class', 'Parent', 'Example', 'Source', 'RequiredAddon',
           'CurrentCapacity', 'CurrentAtSpawn', 'Multiplier']


def defining_addons(pbos, game_folder):
    """Class (lower case) -> (CfgPatches name, source) of the first PBO that defines it."""
    owner = {}
    # Server folders hold the mod folders too, so only the game's own dta and addons are vanilla
    game = os.path.normcase(os.path.abspath(game_folder))
    vanilla = {os.path.join(game, 'dta'), os.path.join(game, 'addons')}
    for pbo in pbos:
        label = 'Vanilla' if os.path.dirname(os.path.normcase(os.path.abspath(pbo))) in vanilla else source_label(pbo)
        for cfg in read_configs(pbo):
            patches = [k for k in next((v for k, v in cfg.items() if k.lower() == 'cfgpatches'), {})
                       if not k.startswith('__')]
            section = next((v for k, v in cfg.items() if k.lower() == 'cfgvehicles'), {})
            for name, body in section.items():
                if isinstance(body, dict) and body.get('__parent') and name.lower() not in owner and patches:
                    owner[name.lower()] = (patches[0], label)
    return owner


def energy(classes, cls):
    """The item's EnergyManager values, following parents."""
    em, _ = resolve(classes, cls, 'EnergyManager', {})
    return em if isinstance(em, dict) else {}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(HERE, 'batteries.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    strings = load_stringtables(pbos)
    classes = load_classes(pbos)
    owner = defining_addons(pbos, args.folders[0])
    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    def display_name(cls):
        name, _ = resolve(classes, cls, 'displayName', '')
        if isinstance(name, str) and name.startswith('$'):
            name = strings.get(name[1:].lower(), name)
        return name if name and name.upper() != 'TODO' else classes[cls]['name']

    rows = []
    for cls, entry in classes.items():
        em = entry['props'].get('EnergyManager')
        # Only classes that set their own capacity, and only stores other items draw from
        # (batteries, gas canisters), not devices with a built-in charge (chainsaw, torch)
        if not isinstance(em, dict) or 'energyStorageMax' not in em or cls not in owner:
            continue
        if not energy(classes, cls).get('isPassiveDevice') or float(em['energyStorageMax']) <= 0:
            continue
        if resolve(classes, cls, 'scope', 0)[0] != 2:
            continue
        parent = entry['parent']
        is_battery = any('battery' in a for a in ancestors(classes, cls))
        rows.append({
            'Type': 'Battery' if is_battery else 'Gas',
            'Class': entry['name'],
            'Parent': classes[parent]['name'] if parent in classes else parent,
            'Example': display_name(cls),
            'Source': owner[cls][1],
            'RequiredAddon': owner[cls][0],
            'CurrentCapacity': round(float(em['energyStorageMax']), 4),
            'CurrentAtSpawn': round(float(energy(classes, cls).get('energyAtSpawn', 0)), 4),
            'Multiplier': old.get(cls, {}).get('Multiplier', ''),
        })

    rows.sort(key=lambda r: (r['Type'] != 'Battery', r['Source'] != 'Vanilla', r['Source'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS)
        w.writeheader()
        w.writerows(rows)
    print(f'{len(rows)} rows written to {args.output}')


if __name__ == '__main__':
    main()
