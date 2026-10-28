"""Builds attacks.csv: every attack infected (zombies) use and how much damage it does.

A zombie hit is a CfgAmmo class (MeleeInfected, MeleeInfectedLong...) named by the zombie's
AttackActions in CfgVehicles. Its DamageApplied sets the Health, Blood and Shock it takes off a
player, how likely it is to start bleeding (bleedThreshold) and how much it wears the clothing
it hits (armorDamage). This lists every attack class an infected actually uses.

Re-running keeps the multipliers you already set.

Usage:
    python scan_attacks.py <DayZ or DayZServer folder> [mod folder ...] [-o attacks.csv]
"""
import argparse, csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'tools'))
from dayz_config import ancestors, load_classes, read_configs, resolve
from mod_scan import find_pbos, source_label

COLUMNS = ['Class', 'Parent', 'Attacks', 'Zombies', 'Source', 'RequiredAddon', 'DamageType',
           'CurrentHealth', 'CurrentBlood', 'CurrentShock', 'CurrentBleedThreshold',
           'CurrentClothing', 'Health', 'Blood', 'Shock', 'Bleed', 'Clothing']
OVERRIDES = ['Health', 'Blood', 'Shock', 'Bleed', 'Clothing']


def defining_addons(pbos, section_name):
    """{class_lower: (first CfgPatches class of the PBO that defines it, source label)}"""
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


def attacks_used(vehicles):
    """{ammo_lower: (set of attack names, set of zombie classes)} for every infected."""
    used = {}
    for cls in vehicles:
        if 'dayzinfected' not in ancestors(vehicles, cls) or resolve(vehicles, cls, 'scope', 0)[0] != 2:
            continue
        actions, _ = resolve(vehicles, cls, 'AttackActions', {})
        found = [(name, body.get('ammoType')) for name, body in actions.items()
                 if isinstance(body, dict) and body.get('ammoType')]
        melee, _ = resolve(vehicles, cls, 'meleeAmmo')
        if melee:
            found.append(('meleeAmmo', melee))
        for action, ammo in found:
            names, zombies = used.setdefault(ammo.lower(), (set(), set()))
            names.add(action)
            zombies.add(cls)
    return used


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('folders', nargs='+', help='DayZ/DayZServer folder first, then mod folders')
    ap.add_argument('-o', '--output', default=os.path.join(HERE, 'attacks.csv'))
    args = ap.parse_args()

    pbos = find_pbos(args.folders)
    old = {}
    if os.path.exists(args.output):
        with open(args.output, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    ammo = load_classes(pbos, 'CfgAmmo')
    owner = defining_addons(pbos, 'CfgAmmo')
    rows = []
    for cls, (actions, zombies) in attacks_used(load_classes(pbos, 'CfgVehicles')).items():
        damage, _ = resolve(ammo, cls, 'DamageApplied')
        if cls not in ammo or cls not in owner or not damage:
            print(f'Skipped {cls}: no DamageApplied found')
            continue
        parent = ammo[cls]['parent']
        part = lambda zone, key='damage': (damage.get(zone) or {}).get(key, 0)
        rows.append({
            'Class': ammo[cls]['name'],
            'Parent': ammo[parent]['name'] if parent in ammo else parent,
            'Attacks': ' '.join(sorted(actions)),
            'Zombies': len(zombies),
            'Source': owner[cls][1],
            'RequiredAddon': owner[cls][0],
            'DamageType': damage.get('type', ''),
            'CurrentHealth': round(float(part('Health')), 4),
            'CurrentBlood': round(float(part('Blood')), 4),
            'CurrentShock': round(float(part('Shock')), 4),
            'CurrentBleedThreshold': round(float(damage.get('bleedThreshold', 0)), 4),
            'CurrentClothing': round(float(part('Health', 'armorDamage')), 4),
            **{k: old.get(cls, {}).get(k, '') for k in OVERRIDES},
        })

    rows.sort(key=lambda r: (r['Source'] != 'Vanilla', r['Source'], -r['Zombies'], r['Class'].lower()))
    with open(args.output, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, COLUMNS)
        w.writeheader()
        w.writerows(rows)
    print(f'{len(rows)} attacks written to {args.output}')


if __name__ == '__main__':
    main()
