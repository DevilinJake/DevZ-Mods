"""Shared helpers for the scanners: finding PBOs and naming where they come from."""
import glob, os, re


def is_game_folder(folder):
    return os.path.isdir(os.path.join(folder, 'dta'))


def game_dlc_folders(folder):
    """The game's own extra content folders (nasdara, sakhal...): a subfolder with addons\\*.pbo
    that isn't a mod (@...) or the Workshop link folder (!Workshop)."""
    return [d for d in sorted(glob.glob(os.path.join(folder, '*')))
            if os.path.basename(d)[:1] not in '@!' and glob.glob(os.path.join(d, 'addons', '*.pbo'))]


def find_pbos(folders):
    """Game PBOs first (dta + addons + DLC folders), then each mod's, in the order given = load order."""
    pbos = []
    for folder in folders:
        subs = [os.path.join(folder, sub) for sub in ('dta', 'addons', 'Addons')]
        if is_game_folder(folder):
            subs += [os.path.join(d, 'addons') for d in game_dlc_folders(folder)]
        for sub in subs:
            found = sorted(glob.glob(os.path.join(sub, '*.pbo')))
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
        # The game's own addons, or a DLC folder inside the game (nasdara\addons)
        if is_game_folder(folder) or is_game_folder(os.path.dirname(folder)):
            return 'Vanilla'
        meta = os.path.join(folder, 'meta.cpp')
        if os.path.exists(meta):
            m = re.search(r'name\s*=\s*"([^"]+)"', open(meta, encoding='utf-8', errors='replace').read())
            if m:
                return m.group(1)
        if os.path.basename(folder) not in ('DayZ', 'DayZServer'):
            return os.path.basename(folder)
    return 'Vanilla'
