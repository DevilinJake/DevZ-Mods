"""Shared helpers for the scanners: finding PBOs and naming where they come from."""
import glob, os, re


def find_pbos(folders):
    """Game PBOs first (dta + addons), then each mod's, in the order given = load order."""
    pbos = []
    for folder in folders:
        for sub in ('dta', 'addons', 'Addons'):
            found = sorted(glob.glob(os.path.join(folder, sub, '*.pbo')))
            pbos += [p for p in found if p not in pbos]
    return pbos


def source_label(pbo):
    """'Vanilla', the mod folder name without the @, or for a Workshop download its name."""
    parts = os.path.normpath(pbo).split(os.sep)
    mod = next((p for p in reversed(parts) if p.startswith('@')), None)
    if mod:
        return mod[1:]
    if 'addons' in (p.lower() for p in parts):
        # Steam Workshop folders are numbered: <id>\meta.cpp has the mod's name
        folder = os.path.dirname(os.path.dirname(pbo))
        meta = os.path.join(folder, 'meta.cpp')
        if os.path.exists(meta):
            m = re.search(r'name\s*=\s*"([^"]+)"', open(meta, encoding='utf-8', errors='replace').read())
            if m:
                return m.group(1)
        if os.path.basename(folder) not in ('DayZ', 'DayZServer'):
            return os.path.basename(folder)
    return 'Vanilla'
