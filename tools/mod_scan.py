"""Shared helpers for the scanners: finding PBOs and naming where they come from."""
import glob, os


def find_pbos(folders):
    """Game PBOs first (dta + addons), then each mod's, in the order given = load order."""
    pbos = []
    for folder in folders:
        for sub in ('dta', 'addons', 'Addons'):
            found = sorted(glob.glob(os.path.join(folder, sub, '*.pbo')))
            pbos += [p for p in found if p not in pbos]
    return pbos


def source_label(pbo):
    """'Vanilla', or the mod folder name without the @."""
    parts = os.path.normpath(pbo).split(os.sep)
    mod = next((p for p in reversed(parts) if p.startswith('@')), None)
    return mod[1:] if mod else 'Vanilla'
