"""Draws a DevZ Workshop preview card (512x512 PNG). Used by make_workshop.py.

Needs Pillow and numpy. Everything is drawn here: a moody dark background, a pictogram of
what the mod does, the title in big condensed type and a DevZ tag. The only picture files are
traced white-on-black outlines: ak_silhouette.png (a suppressed AKM), heli_silhouette.png (an
MH-6 Little Bird) and landrover_silhouette.png (a Defender 110).

    from workshop_preview import card
    card(["Ammo", "Stacks"], (214, 157, 70), "ammo", seed=10, path="preview.png")

Pictograms: inventory, ammo, weapon, combine, recipes, repair, autostack, infected, transfer, crash,
autorun.
A bundle card takes a list of (pictogram, accent) pairs instead of a name and draws one small
tile per mod, each in that mod's colour.
"""
import math
import random
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

HERE = Path(__file__).resolve().parent
FONTS = Path("C:/Windows/Fonts")
S = 1024          # drawn at this size, saved at FINAL (smoother edges)
FINAL = 512

WHITE = (240, 240, 236)
DARK = (22, 24, 28)
BRASS = (214, 172, 88)
COPPER = (176, 112, 72)
COND = [(204, 66, 60), (226, 140, 52), (214, 196, 72), (96, 196, 96)]   # DayZ condition colours


# ----------------------------------------------------------------------------- fonts
def font(name, size, variation=None):
    f = ImageFont.truetype(str(FONTS / name), size)
    if variation:
        try:
            f.set_variation_by_name(variation)
        except Exception:
            f.set_variation_by_name(variation.encode())
    return f


def bahn(size, var="Bold Condensed"):
    return font("bahnschrift.ttf", size, var)


def segoe(size):
    return font("segoeui.ttf", size)


# ----------------------------------------------------------------------------- textures
def smooth_noise(w, h, cells, seed):
    rng = np.random.default_rng(seed)
    small = (rng.random((cells, cells)) * 255).astype("uint8")
    im = Image.fromarray(small).resize((w, h), Image.BICUBIC)
    return np.asarray(im).astype(np.float32) / 255.0


def fbm(w, h, seed, octaves=(3, 6, 12, 24, 48), weights=(1, .6, .35, .2, .1)):
    acc = np.zeros((h, w), np.float32)
    tot = 0
    for i, (c, wt) in enumerate(zip(octaves, weights)):
        acc += wt * smooth_noise(w, h, c, seed * 7 + i)
        tot += wt
    return acc / tot


def grain(w, h, seed, amount=10):
    rng = np.random.default_rng(seed)
    return (rng.random((h, w)) - 0.5) * 2 * amount


def vignette(w, h, strength=0.55, power=2.2):
    y, x = np.mgrid[0:h, 0:w].astype(np.float32)
    dx = (x - w / 2) / (w / 2)
    dy = (y - h / 2) / (h / 2)
    r = np.sqrt(dx * dx + dy * dy) / math.sqrt(2)
    return 1 - strength * np.power(np.clip(r, 0, 1), power)


def to_img(arr):
    return Image.fromarray(np.clip(arr, 0, 255).astype("uint8"), "RGB")


def solid(w, h, color):
    return np.ones((h, w, 3), np.float32) * np.array(color, np.float32)


def mix(base, color, mask):
    m = mask[..., None]
    return base * (1 - m) + np.array(color, np.float32) * m


def radial(w, h, cx, cy, radius):
    y, x = np.mgrid[0:h, 0:w].astype(np.float32)
    d = np.sqrt((x - cx) ** 2 + (y - cy) ** 2) / radius
    return np.clip(1 - d, 0, 1) ** 1.6


def scratches(img, seed, n=40, color=(255, 255, 255), alpha=22):
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    rnd = random.Random(seed)
    w, h = img.size
    for _ in range(n):
        x0, y0 = rnd.uniform(0, w), rnd.uniform(0, h)
        ang = rnd.uniform(-0.5, 0.5) + rnd.choice([0, math.pi / 2])
        ln = rnd.uniform(40, 400)
        x1, y1 = x0 + math.cos(ang) * ln, y0 + math.sin(ang) * ln
        d.line([x0, y0, x1, y1], fill=color + (rnd.randint(alpha // 2, alpha),), width=rnd.choice([1, 1, 2]))
    img.alpha_composite(layer)
    return img


def darken(c, k):
    return tuple(int(v * k) for v in c)


def lighten(c, k):
    return tuple(int(v + (255 - v) * k) for v in c)


# ----------------------------------------------------------------------------- pictograms
class Pict:
    """Draws a pictogram into a square RGBA layer. Coordinates are in 0..100 units."""

    def __init__(self, size, fg, accent):
        self.size = size
        self.img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        self.d = ImageDraw.Draw(self.img)
        self.fg, self.accent = fg, accent
        self.k = size / 100

    def p(self, *xy):
        return [v * self.k for v in xy]

    def lw(self, w):
        return max(1, int(w * self.k))

    # --- primitives
    def slot(self, x, y, w, fill=None, border=None, qty=None, qty_color=None, radius=6):
        """DayZ-style inventory slot: rounded square, optional quantity bar."""
        border = border or self.fg
        fill = fill or (0, 0, 0, 90)
        self.d.rounded_rectangle(self.p(x, y, x + w, y + w), radius=radius * self.k, fill=fill,
                                 outline=border, width=self.lw(2.2))
        if qty is not None:
            bx0, by0 = x + w * 0.12, y + w * 0.74
            bw, bh = w * 0.76, w * 0.12
            self.d.rounded_rectangle(self.p(bx0, by0, bx0 + bw, by0 + bh), radius=2 * self.k, fill=(0, 0, 0, 160))
            if qty > 0:
                self.d.rounded_rectangle(self.p(bx0, by0, bx0 + bw * qty, by0 + bh), radius=2 * self.k,
                                         fill=qty_color or self.accent)

    def arrow(self, x0, y, x1, color=None, w=3.2):
        color = color or self.fg
        self.d.line(self.p(x0, y, x1 - 2, y), fill=color, width=self.lw(w))
        h = w * 1.9
        self.d.polygon(self.p(x1, y, x1 - h * 1.1, y - h, x1 - h * 1.1, y + h), fill=color)

    def plus(self, x, y, s=5, color=None, w=3):
        color = color or self.fg
        self.d.line(self.p(x - s, y, x + s, y), fill=color, width=self.lw(w))
        self.d.line(self.p(x, y - s, x, y + s), fill=color, width=self.lw(w))

    def tick(self, x, y, s=6, color=None, w=4):
        color = color or self.fg
        self.d.line(self.p(x - s, y, x - s * 0.25, y + s * 0.7, x + s * 1.1, y - s * 0.9), fill=color,
                    width=self.lw(w), joint="curve")

    def no_sign(self, x, y, r, color, w=3.5):
        self.d.ellipse(self.p(x - r, y - r, x + r, y + r), outline=color, width=self.lw(w))
        o = r * 0.68
        self.d.line(self.p(x - o, y - o, x + o, y + o), fill=color, width=self.lw(w))

    def cartridge(self, x, y, w, h, case=BRASS, tip=COPPER):
        self.d.rectangle(self.p(x - w * 0.06, y + h * 0.93, x + w * 1.06, y + h), fill=case)      # rim
        self.d.rectangle(self.p(x, y + h * 0.42, x + w, y + h * 0.93), fill=case)                  # case
        self.d.polygon(self.p(x, y + h * 0.42, x + w, y + h * 0.42, x + w * 0.85, y + h * 0.34,
                              x + w * 0.15, y + h * 0.34), fill=case)                              # shoulder
        self.d.polygon(self.p(x + w * 0.15, y + h * 0.34, x + w * 0.85, y + h * 0.34, x + w * 0.85, y + h * 0.18,
                              x + w * 0.5, y, x + w * 0.15, y + h * 0.18), fill=tip)               # bullet
        self.d.rectangle(self.p(x + w * 0.18, y + h * 0.46, x + w * 0.3, y + h * 0.9), fill=lighten(case, 0.35))

    def backpack(self, x, y, w, h, fill, outline, cells):
        r = w * 0.18
        self.d.rounded_rectangle(self.p(x + w * 0.08, y, x + w * 0.92, y + h * 0.3), radius=r * self.k,
                                 fill=fill, outline=outline, width=self.lw(2.2))                   # lid
        self.d.rounded_rectangle(self.p(x, y + h * 0.18, x + w, y + h), radius=r * self.k, fill=fill,
                                 outline=outline, width=self.lw(2.2))                              # body
        self.d.rounded_rectangle(self.p(x + w * 0.36, y - h * 0.08, x + w * 0.64, y + h * 0.02),
                                 radius=3 * self.k, outline=outline, width=self.lw(2))             # handle
        cols, rows = cells
        gx, gy, gw, gh = x + w * 0.14, y + h * 0.36, w * 0.72, h * 0.52
        cw, ch = gw / cols, gh / rows
        for i in range(cols):
            for j in range(rows):
                self.d.rectangle(self.p(gx + i * cw + 0.8, gy + j * ch + 0.8, gx + (i + 1) * cw - 0.8,
                                        gy + (j + 1) * ch - 0.8), fill=self.accent)

    def wrench(self, x, y, L, color, angle=-45):
        size = int(L * self.k * 1.6)
        lay = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        d = ImageDraw.Draw(lay)
        k = L * self.k
        cx = size / 2
        head_r = k * 0.19
        d.ellipse([cx - head_r, size * 0.2 - head_r, cx + head_r, size * 0.2 + head_r], fill=color)
        d.rounded_rectangle([cx - k * 0.07, size * 0.2, cx + k * 0.07, size * 0.2 + k * 0.62],
                            radius=k * 0.05, fill=color)
        d.rectangle([cx - head_r * 0.45, size * 0.2 - head_r * 1.2, cx + head_r * 0.45, size * 0.2 + head_r * 0.15],
                    fill=(0, 0, 0, 0))
        lay = lay.rotate(angle, resample=Image.BICUBIC)
        self.img.alpha_composite(lay, (int(x * self.k - size / 2), int(y * self.k - size / 2)))

    def shield(self, x, y, w, fill, outline):
        h = w * 1.15
        pts = self.p(x, y + h * 0.12, x + w * 0.5, y, x + w, y + h * 0.12, x + w, y + h * 0.5,
                     x + w * 0.5, y + h, x, y + h * 0.5)
        self.d.polygon(pts, fill=fill, outline=outline, width=self.lw(2.5))

    def traced(self, mask_path, x, y, w, color):
        """Paste a traced silhouette (white-on-black mask image) scaled to width w units."""
        m = Image.open(mask_path).convert("L")
        pw = int(w * self.k)
        ph = int(round(m.height * pw / m.width))
        m = m.resize((pw, ph), Image.LANCZOS)
        layer = Image.new("RGBA", m.size, color[:3] + (255,))
        layer.putalpha(m)
        self.img.alpha_composite(layer, (int(x * self.k), int(y * self.k)))

    # --- the pictograms
    def inventory(self):
        """small pack -> big pack"""
        fill = (0, 0, 0, 80)
        self.backpack(8, 36, 24, 32, fill, self.fg, cells=(2, 2))
        self.arrow(38, 56, 56, color=self.accent, w=3.5)
        self.backpack(60, 18, 36, 50, fill, self.fg, cells=(3, 3))

    def ammo(self):
        """two rows of cartridges with a x2 badge"""
        w, h = 11, 44
        for x in [17, 31, 45, 59]:
            self.cartridge(x, 40, w, h, case=darken(BRASS, 0.62), tip=darken(COPPER, 0.62))
        for x in [10, 24, 38, 52, 66]:
            self.cartridge(x, 52, w, h)
        r, cx, cy = 15, 84, 24
        self.d.ellipse(self.p(cx - r, cy - r, cx + r, cy + r), fill=self.accent)
        self.d.text(self.p(cx, cy + 0.5), "\u00d72", font=bahn(int(17 * self.k), "Bold"), fill=DARK, anchor="mm")

    def weapon(self):
        """suppressed AK (traced outline) with a shield-and-tick badge"""
        self.traced(HERE / "ak_silhouette.png", 0, 24, 100, self.fg)
        # the shield tapers, so its visual centre is above its box centre: centre the tick's
        # bounding box on the shield's centroid (81, 64.4)
        self.shield(66, 50, 30, self.accent, self.fg)
        s = 6.5
        self.tick(81 - 0.05 * s, 64.4 + 0.1 * s, s=s, color=DARK, w=4.5)

    def combine(self):
        """half + half -> full; splitting crossed out"""
        self.slot(4, 20, 24, qty=0.5)
        self.plus(37, 32, s=5)
        self.slot(46, 20, 24, qty=0.5)
        self.arrow(76, 32, 92, color=self.accent, w=3)
        self.slot(28, 56, 28, qty=1.0, border=self.accent)
        self.slot(66, 60, 14, qty=0.5)
        self.slot(84, 60, 14, qty=0.5)
        self.no_sign(82, 67, 18, (214, 80, 80), w=3.2)

    def recipes(self):
        """a right-click recipe row, crossed out"""
        self.slot(4, 36, 22)
        self.plus(33, 47, s=4.5)
        self.slot(40, 36, 22)
        self.arrow(68, 47, 80, w=3)
        self.slot(80, 36, 18, border=self.accent)
        self.d.line(self.p(8, 54, 22, 40), fill=self.fg, width=self.lw(2.4))          # knife
        self.d.line(self.p(8, 54, 12, 50), fill=self.accent, width=self.lw(4))
        self.d.rounded_rectangle(self.p(45, 42, 57, 52), radius=2 * self.k, outline=self.fg, width=self.lw(2))  # rag
        self.no_sign(50, 48, 44, (214, 80, 80), w=4.5)

    def repair(self):
        """condition dots red -> green, wrench and sewing kit"""
        xs, rs = [10, 30, 52, 80], [5, 6, 7, 13]
        for i, (x, r) in enumerate(zip(xs, rs)):
            self.d.ellipse(self.p(x - r, 32 - r, x + r, 32 + r), fill=COND[i])
            if i < 3:
                self.d.line(self.p(x + r + 2, 32, xs[i + 1] - rs[i + 1] - 2, 32), fill=self.fg, width=self.lw(2))
        self.tick(79, 32, s=6, color=DARK, w=4.5)
        self.wrench(30, 70, 40, self.fg, angle=-40)
        self.slot(60, 56, 28, qty=0.8, border=self.fg)
        self.d.line(self.p(66, 76, 82, 60), fill=self.fg, width=self.lw(2))             # needle
        self.d.ellipse(self.p(80, 58, 84, 62), outline=self.fg, width=self.lw(1.4))

    def autostack(self):
        """two loose groups -> one stack"""
        w, h = 8, 32
        for x in [6, 16]:
            self.cartridge(x, 44, w, h)
        self.plus(30, 60, s=4)
        for x in [36, 46, 56]:
            self.cartridge(x, 44, w, h)
        self.arrow(26, 22, 70, color=self.accent, w=3)
        # the merged stack, lower right, with a count badge
        for x in [72, 82, 92]:
            self.cartridge(x, 52, w, h)
        for x in [67, 77, 87]:
            self.cartridge(x, 60, w, h)
        r, cx, cy = 11, 86, 30
        self.d.ellipse(self.p(cx - r, cy - r, cx + r, cy + r), fill=self.accent)
        self.d.text(self.p(cx, cy + 0.5), "52", font=bahn(int(13 * self.k), "Bold"), fill=DARK, anchor="mm")

    def infected(self):
        """three claw slashes over a health bar"""
        for i, x in enumerate([22, 42, 62]):
            self.d.polygon(self.p(x, 8, x + 8, 8, x + 28, 56, x + 14, 56), fill=self.accent)
        # health bar underneath, mostly drained, red
        self.d.rounded_rectangle(self.p(8, 70, 92, 84), radius=4 * self.k, fill=(0, 0, 0, 150),
                                 outline=self.fg, width=self.lw(2))
        self.d.rounded_rectangle(self.p(10, 72, 38, 82), radius=3 * self.k, fill=(204, 66, 60))
        r, cx, cy = 11, 86, 26
        self.d.ellipse(self.p(cx - r, cy - r, cx + r, cy + r), fill=self.fg)
        self.d.text(self.p(cx, cy + 0.5), "1.5×", font=bahn(int(10 * self.k), "Bold"), fill=DARK, anchor="mm")

    def transfer(self):
        """a cursor sweeping over a row of items, which fly into a crate"""
        # your inventory: a row of slots, the first two already emptied by the sweep
        for i, x in enumerate([4, 22, 40]):
            filled = i == 2
            self.slot(x, 20, 15, fill=self.accent if filled else None, border=self.fg if filled else self.fg + (110,), radius=3)
        # the sweep: a dashed trail along the row, then the mouse pointer
        for x in range(6, 44, 6):
            self.d.line(self.p(x, 42, x + 3, 42), fill=self.accent, width=self.lw(2.2))
        cx, cy = 50, 30
        self.d.polygon(self.p(cx, cy, cx, cy + 20, cx + 5, cy + 15, cx + 9, cy + 23, cx + 12, cy + 21,
                              cx + 8, cy + 14, cx + 14, cy + 14), fill=self.fg, outline=DARK)
        self.arrow(30, 62, 56, color=self.accent, w=3.2)
        # the crate, its slots filling up
        self.d.rounded_rectangle(self.p(60, 44, 98, 90), radius=4 * self.k, fill=(0, 0, 0, 110),
                                 outline=self.fg, width=self.lw(2.4))
        for j, y in enumerate([50, 70]):
            for i, x in enumerate([64, 81]):
                self.slot(x, y, 13, fill=self.accent if (i + 2 * j) < 2 else None, radius=3)

    def silhouette(self, mask_path, w, color, at, anchor=(0.5, 1.0), angle=0, halo=0):
        """Paste a traced silhouette (white-on-black mask) scaled to width w units, rotated by
        angle degrees (positive tips the left end down), so that the mask point `anchor`
        (fractions of its width and height) lands on `at` (units). halo: width in units of a
        dark keyline around it, to keep it apart from whatever is behind."""
        m = Image.open(mask_path).convert("L")
        s = int(w * self.k) / m.width
        m = m.resize((int(w * self.k), int(round(m.height * s))), Image.LANCZOS)
        pad = int(halo * self.k) + 2
        big = Image.new("L", (m.width + 2 * pad, m.height + 2 * pad), 0)
        big.paste(m, (pad, pad))
        m = big
        ax, ay = anchor[0] * (m.width - 2 * pad) + pad, anchor[1] * (m.height - 2 * pad) + pad
        layers = [(m, color)]
        if halo:
            layers.insert(0, (m.filter(ImageFilter.MaxFilter(int(halo * self.k) | 1)), DARK))
        dot = Image.new("L", m.size, 0)                      # follows the anchor through the rotation
        ImageDraw.Draw(dot).ellipse([ax - 3, ay - 3, ax + 3, ay + 3], fill=255)
        if angle:
            dot = dot.rotate(angle, resample=Image.BICUBIC, expand=True)
        ys, xs = np.nonzero(np.asarray(dot) > 100)
        off = (int(at[0] * self.k - xs.mean()), int(at[1] * self.k - ys.mean()))
        for mask, col in layers:
            lay = Image.new("RGBA", mask.size, col[:3] + (255,))
            lay.putalpha(mask)
            if angle:
                lay = lay.rotate(angle, resample=Image.BICUBIC, expand=True)
            self.img.alpha_composite(lay, off)

    def crash(self):
        """a Little Bird tipped onto its nose on landing and, apart from it, a Land Rover into
        a wall; the hull bar drained to 10% but held there by a stop (Badly Damaged, never
        Ruined). Drawn on a 720 layer (see card()) so the two crashes fit with a gap between."""
        ground = 60
        # the heli, nose down, resting on the front of its skids
        self.silhouette(HERE / "heli_silhouette.png", 44, self.fg, at=(15, ground),
                        anchor=(0.09, 0.99), angle=25)
        cut = int((ground + 0.5) * self.k)
        self.img.paste(Image.new("RGBA", (self.size, self.size - cut), (0, 0, 0, 0)), (0, cut))
        # its nose: the leftmost solid pixel just above the ground line
        a = np.asarray(self.img.getchannel("A"))[int((ground - 6) * self.k):cut]
        nose = np.nonzero(a.max(axis=0) > 100)[0].min() / self.k
        self.d.line(self.p(1, ground, 99, ground), fill=self.fg + (170,), width=self.lw(1.8))
        # the Land Rover, its bumper against a wall of blocks at the right edge
        self.silhouette(HERE / "landrover_silhouette.png", 36, self.fg, at=(72, ground))
        wall, bumper = 94, 90
        for i in range(3):
            y = ground - (i + 1) * 6
            self.d.rectangle(self.p(wall, y, 99, y + 6), fill=(0, 0, 0, 90), outline=self.fg, width=self.lw(2))
        for dx0, dy0, dx1, dy1 in [(0.5, -9, 2.5, -13), (1, -5.5, 4, -6), (0.5, -2, 2.5, 2)]:
            self.d.line(self.p(bumper + dx0, ground + dy0, bumper + dx1, ground + dy1), fill=self.accent,
                        width=self.lw(2.6))
        # impact strokes fanning out ahead of the heli's nose
        ix, iy = nose - 1, ground - 2
        for dx0, dy0, dx1, dy1 in [(-1, -4, -3.5, -10), (-3, -1.5, -10, -4), (-3, 2, -9, 5)]:
            self.d.line(self.p(ix + dx0, iy + dy0, ix + dx1, iy + dy1), fill=self.accent, width=self.lw(3))
        # hull bar: drained, but a stop holds it at 10%
        self.d.rounded_rectangle(self.p(12, 72, 88, 86), radius=4 * self.k, fill=(0, 0, 0, 150),
                                 outline=self.fg, width=self.lw(2))
        self.d.rounded_rectangle(self.p(14, 74, 27, 84), radius=3 * self.k, fill=self.accent)
        self.d.rounded_rectangle(self.p(26.5, 67, 30, 91), radius=1.5 * self.k, fill=self.fg)

    def autorun(self):
        """a Z keycap, a running figure with speed lines trailing it, an arrow ahead"""
        # the keycap, lower left: a dark key with a lighter top face and a Z
        self.d.rounded_rectangle(self.p(6, 58, 34, 88), radius=5 * self.k, fill=darken(self.fg, 0.6),
                                 outline=self.fg, width=self.lw(2))
        self.d.rounded_rectangle(self.p(9, 60, 31, 82), radius=4 * self.k, fill=(0, 0, 0, 120),
                                 outline=self.fg + (120,), width=self.lw(1.2))
        self.d.text(self.p(20, 71.5), "Z", font=bahn(int(17 * self.k), "Bold"), fill=self.fg, anchor="mm")
        # the runner, mid-stride, leaning into the run (lines in 0..100 units)
        w = self.lw(4.6)
        hx, hy = 66, 18
        self.d.ellipse(self.p(hx - 6, hy - 6, hx + 6, hy + 6), fill=self.fg)
        self.d.line(self.p(63, 26, 55, 50), fill=self.fg, width=w, joint="curve")            # torso
        self.d.line(self.p(61, 32, 72, 38, 80, 30), fill=self.fg, width=w, joint="curve")    # front arm
        self.d.line(self.p(61, 32, 50, 36, 44, 28), fill=self.fg, width=w, joint="curve")    # back arm
        self.d.line(self.p(55, 50, 70, 58, 76, 74), fill=self.fg, width=w, joint="curve")    # front leg
        self.d.line(self.p(55, 50, 45, 62, 36, 60), fill=self.fg, width=w, joint="curve")    # back leg
        # speed lines trailing the runner
        for x0, x1, y in [(26, 38, 30), (22, 36, 40), (30, 40, 50)]:
            self.d.line(self.p(x0, y, x1, y), fill=self.accent, width=self.lw(2.6))
        # ground and the way ahead
        self.d.line(self.p(40, 78, 98, 78), fill=self.fg + (150,), width=self.lw(1.8))
        self.arrow(48, 90, 96, color=self.accent, w=3.4)

    def tiles(self, items):
        """A bundle: one small card-like tile per mod, each with that mod's own pictogram and
        accent colour. items: list of (pictogram name, accent). Laid out 3 across, then the rest
        on a second row, centred."""
        per_row, gap = 3, 4.5
        rows = [items[i:i + per_row] for i in range(0, len(items), per_row)]
        tile = (100 - gap * (per_row - 1)) / per_row
        pad = tile * 0.10
        total_h = tile * len(rows) + gap * (len(rows) - 1)
        top = (100 - total_h) / 2
        for r, row in enumerate(rows):
            left = (100 - (tile * len(row) + gap * (len(row) - 1))) / 2
            for c, (name, accent) in enumerate(row):
                x, y = left + c * (tile + gap), top + r * (tile + gap)
                # tile: dark glass tinted with the mod's colour, a thin light edge
                tint = tuple(int(v * 0.22) for v in accent) + (200,)
                self.d.rounded_rectangle(self.p(x, y, x + tile, y + tile), radius=tile * 0.1 * self.k,
                                         fill=tint, outline=self.fg + (95,), width=self.lw(0.7))
                # the little accent rule at the bottom, like the big card's footer rule
                self.d.rectangle(self.p(x + pad, y + tile - pad * 0.9, x + tile * 0.38, y + tile - pad * 0.9 + 1.4),
                                 fill=accent)
                inner = int((tile - 2 * pad) * self.k)
                layer = Pict(inner, self.fg, accent).draw(name)
                ox = int((x + pad) * self.k)
                oy = int((y + pad * 0.55) * self.k)
                self.img.alpha_composite(layer, (ox, oy))

    def draw(self, name):
        if isinstance(name, (list, tuple)):
            self.tiles(name)
        else:
            getattr(self, name)()
        return self.img


def pictogram(name, size, fg, accent):
    return Pict(size, fg, accent).draw(name)


# ----------------------------------------------------------------------------- compositing
def paste_with_shadow(canvas, layer, xy, blur=14, offset=(0, 10), alpha=150):
    sh = Image.new("RGBA", layer.size, (0, 0, 0, 0))
    sh.putalpha(layer.getchannel("A").point(lambda v: v * alpha // 255))
    sh = sh.filter(ImageFilter.GaussianBlur(blur))
    canvas.alpha_composite(sh, (xy[0] + offset[0], xy[1] + offset[1]))
    canvas.alpha_composite(layer, xy)


def centered(layer, cy, cx=S // 2):
    return (int(cx - layer.width / 2), int(cy - layer.height / 2))


# ----------------------------------------------------------------------------- the card
def card(title, accent, pict, seed, path, footer="by DevilinJake"):
    """title: list of 2 or 3 lines. accent: RGB. pict: pictogram name. seed: fixes the texture.
    footer: the small text after the DevZ tag."""
    w = h = S
    base = solid(w, h, (34, 40, 36))
    n1 = fbm(w, h, seed, octaves=(2, 4, 8, 16), weights=(1, .7, .4, .2))
    base = mix(base, (62, 74, 60), np.clip((n1 - 0.35) * 1.6, 0, 1) * 0.9)      # mossy patches
    base = mix(base, (18, 20, 22), np.clip((0.55 - n1) * 1.8, 0, 1) * 0.6)      # dark pockets
    y = np.mgrid[0:h, 0:w][0].astype(np.float32) / h
    base = mix(base, (14, 16, 18), np.clip((y - 0.55) * 2.2, 0, 1) * 0.85)      # darker ground
    base = mix(base, accent, radial(w, h, w * 0.5, h * 0.33, w * 0.42) * 0.22)  # glow behind the pictogram
    base = base * vignette(w, h, 0.6, 2.0)[..., None] + grain(w, h, seed, 7)[..., None]
    img = to_img(base).convert("RGBA")
    img = scratches(img, seed, n=30, alpha=16)

    three = len(title) > 2
    bundle = isinstance(pict, (list, tuple))
    psize = 600 if pict == "weapon" else (470 if three else 520)   # the rifle is wide and short
    if pict == "crash":
        psize = 720                                                 # two crashes side by side, with a gap
    if bundle:
        psize = 720                                                 # a grid of tiles, wider than one pictogram
    p = pictogram(pict, psize, WHITE, accent)
    paste_with_shadow(img, p, centered(p, int(h * (0.29 if three else 0.31))), blur=18, offset=(0, 14), alpha=170)

    # title, laid out upwards from the footer rule so descenders (p, y) never touch it
    d = ImageDraw.Draw(img)
    rule_y = 930
    size = 126 if three else 150
    f = bahn(size)
    _, descent = f.getmetrics()
    baseline = rule_y - 28 - descent
    for line in reversed(title):
        d.text((70, baseline), line, font=f, fill=WHITE, anchor="ls")
        baseline -= int(size * 0.9)
    d.rectangle([70, rule_y, 70 + 150, rule_y + 8], fill=accent)
    d.text((240, 916), "DevZ", font=bahn(40, "Bold"), fill=accent)
    d.text((352, 924), footer, font=segoe(30), fill=(190, 190, 186))

    img.convert("RGB").resize((FINAL, FINAL), Image.LANCZOS).save(path, optimize=True)
