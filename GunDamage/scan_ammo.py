"""Writes ammo.csv: every bullet type the game (and any mods you point it at) defines, with the
guns that fire it and its current damage. Multipliers already in ammo.csv are kept.

Usage:
    python scan_ammo.py "C:/Program Files (x86)/Steam/steamapps/common/DayZServer" [mod folders...]

Every *.pbo under the given folders' addons\\ is read, in the order given (later ones override
earlier ones, like the game's load order).
"""
import csv, glob, json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'tools'))
import dayz_config as dc  # noqa: E402

TABLE = os.path.join(HERE, 'ammo.csv')
FIELDS = ['Class', 'Parent', 'Guns', 'Source', 'RequiredAddon', 'DamageType', 'Dispersion',
          'BleedThreshold', 'DamageOverride', 'CurrentHealth', 'CurrentBlood', 'CurrentShock',
          'Health', 'Shock']


def pbos_in(folders):
    out = []
    for folder in folders:
        out += sorted(glob.glob(os.path.join(folder, 'addons', '*.pbo')))
    return out


def patch_names(pbo):
    names = []
    for cfg in dc.read_configs(pbo):
        patches = next((v for k, v in cfg.items() if k.lower() == 'cfgpatches'), {})
        names += [k for k in patches if not k.startswith('__')]
    return names


def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    pbos = pbos_in(sys.argv[1:])
    ammo = dc.load_classes(pbos, 'CfgAmmo')
    mags = dc.load_classes(pbos, 'CfgMagazines')
    weapons = dc.load_classes(pbos, 'CfgWeapons')

    # Which guns fire each bullet: gun -> cartridge/magazine (chamberableFrom, magazines) -> ammo
    guns = {}
    for key, w in weapons.items():
        if dc.resolve(weapons, key, 'scope')[0] != 2:
            continue
        feeds = (dc.resolve(weapons, key, 'chamberableFrom')[0] or []) + (dc.resolve(weapons, key, 'magazines')[0] or [])
        for feed in feeds:
            bullet = dc.resolve(mags, str(feed).lower(), 'ammo')[0]
            if bullet:
                guns.setdefault(bullet.lower(), set()).add(w['name'])

    old = {}
    if os.path.exists(TABLE):
        with open(TABLE, newline='', encoding='utf-8-sig') as f:
            old = {r['Class'].lower(): r for r in csv.DictReader(f)}

    patches = {}
    rows = []
    for key, a in ammo.items():
        da = a['props'].get('DamageApplied')
        if not da or str(da.get('type', '')).lower() != 'projectile' or key not in guns:
            continue
        if a['source'] not in patches:
            patches[a['source']] = (patch_names(a['source']) or ['DZ_Data'])[0]
        get = lambda zone: float(da.get(zone, {}).get('damage', 0))
        override = da.get('defaultDamageOverride')
        row = {
            'Class': a['name'], 'Parent': ammo.get(a['parent'], {'name': a['parent']})['name'],
            'Guns': ' '.join(sorted(guns[key], key=str.lower)),
            'Source': os.path.basename(a['source']), 'RequiredAddon': patches[a['source']],
            'DamageType': da.get('type', 'Projectile'), 'Dispersion': da.get('dispersion', 0),
            'BleedThreshold': da.get('bleedThreshold', ''),
            'DamageOverride': json.dumps(override) if override else '',
            'CurrentHealth': get('Health'), 'CurrentBlood': get('Blood'), 'CurrentShock': get('Shock'),
            'Health': old.get(key, {}).get('Health', ''), 'Shock': old.get(key, {}).get('Shock', ''),
        }
        rows.append(row)

    with open(TABLE, 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, FIELDS)
        w.writeheader()
        w.writerows(rows)
    print(f'{len(rows)} bullet types written to {TABLE}')


if __name__ == '__main__':
    main()
