"""Draws a DevZ Workshop preview card (512x512 PNG). Used by make_workshop.py.

Needs Pillow and numpy. Everything is drawn here: a moody dark background, a pictogram of
what the mod does, the title in big condensed type and a DevZ tag. The only picture file is
ak_silhouette.png, an outline traced from a photo of a suppressed AKM.

    from workshop_preview import card
    card(["Ammo", "Stacks"], (214, 157, 70), "ammo", seed=10, path="preview.png")

Pictograms: inventory, ammo, weapon, combine, recipes, repair, autostack, infected.
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

    def draw(self, name):
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
    psize = 600 if pict == "weapon" else (470 if three else 520)   # the rifle is wide and short
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
