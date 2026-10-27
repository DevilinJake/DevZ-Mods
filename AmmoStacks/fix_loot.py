"""Keeps the amount of ammo found in loot the same after changing stack sizes.

In types.xml, quantmin/quantmax are a percentage of a full stack. Doubling a stack would double
every ammo pile that spawns, so this scales those percentages down by the same amount.

It edits types.xml in place (a copy is saved once as types.xml.bak) and remembers each item's
original percentages in a comment, so it's safe to run again after changing the stacks; set a
stack back to its original size and its loot percentages go back too.

Usage:
    python fix_loot.py <path to mission\\db\\types.xml>
"""
import argparse, os, re, shutil, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from generate_config import compute_stacks

MARK = re.compile(r'\s*<!-- DevZ_AmmoStacks original quantmin (-?\d+) quantmax (-?\d+) -->')


def fix_block(block, old_stack, new_stack):
    """Return the <type> block with scaled quantmin/quantmax, and whether it changed."""
    mark = MARK.search(block)
    qmin = re.search(r'<quantmin>(-?\d+)</quantmin>', block)
    qmax = re.search(r'<quantmax>(-?\d+)</quantmax>', block)
    if not qmin or not qmax:
        return block, False
    orig_min, orig_max = (int(mark.group(1)), int(mark.group(2))) if mark else (int(qmin.group(1)), int(qmax.group(1)))
    if orig_min < 0 or orig_max < 0:
        return block, False  # -1 means "no quantity set": the game uses its own default

    if new_stack == old_stack:
        new_min, new_max = orig_min, orig_max
    else:
        scale = old_stack / new_stack
        new_min = max(1, round(orig_min * scale)) if orig_min else 0
        new_max = max(new_min, max(1, round(orig_max * scale)) if orig_max else 0)

    out = MARK.sub('', block)
    out = re.sub(r'<quantmin>-?\d+</quantmin>', f'<quantmin>{new_min}</quantmin>', out)
    out = re.sub(r'<quantmax>-?\d+</quantmax>', f'<quantmax>{new_max}</quantmax>', out)
    if new_stack != old_stack:
        out = re.sub(r'(<type name="[^"]+">)',
                     rf'\1\n        <!-- DevZ_AmmoStacks original quantmin {orig_min} quantmax {orig_max} -->', out, count=1)
    return out, out != block


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('types_xml')
    args = ap.parse_args()

    stacks = {r['Class'].lower(): (r['old'], r['new']) for r in compute_stacks()}
    with open(args.types_xml, encoding='utf-8') as f:
        text = f.read()

    changed = []

    def replace(m):
        name = m.group(1).lower()
        if name not in stacks:
            return m.group(0)
        block, did = fix_block(m.group(0), *stacks[name])
        if did:
            changed.append(m.group(1))
        return block

    new_text = re.sub(r'<type name="([^"]+)">.*?</type>', replace, text, flags=re.S)
    if not changed:
        print('Nothing to change.')
        return
    backup = args.types_xml + '.bak'
    if not os.path.exists(backup):
        shutil.copy2(args.types_xml, backup)
    with open(args.types_xml, 'w', encoding='utf-8', newline='') as f:
        f.write(new_text)
    print(f'Loot percentages updated for {len(changed)} ammo types in {args.types_xml}')


if __name__ == '__main__':
    main()
