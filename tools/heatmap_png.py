#!/usr/bin/env python3
"""Render a xpeccy memory heat-map (.heat.csv export) as a PNG.

Input is CSV with a header row and one line per memory cell (every cell,
including untouched all-zero ones):
    type,page,offset,read,write,exec
where type is RAM or ROM, page is the 16k-page number, offset is 0..16383.

Four rendering styles (--style):

  sectors (default) - a schematic map matching the ZX address space, one
    "sector" tile per 256 bytes, arranged 8x8 (configurable via --grid) per
    16k page. Pages that share a CPU slot are grouped side by side on the
    same row:
        0000: [ROM0][ROM1]...   (all ROM pages, none of them is "the" one)
        4000: [RAM5]            (always fixed here)
        8000: [RAM2]            (always fixed here)
        C000: [RAM0][RAM1]...   (RAM0 is just the default occupant, not fixed)
    No gradients: a sector is flat gray if untouched, otherwise split into
    up to 3 solid stripes (green=read, red=write, cyan=exec) for whichever
    access types touched any byte in it.

  gradient - one pixel per byte, `--width` bytes per row, stacked blocks
    (same page grouping as above, just one page per block instead of side
    by side). Color blends from white by log-scaled read/write/exec
    intensity instead of a hard yes/no per sector - useful for spotting
    exactly which bytes in a hot page are busiest.

  detail - one tile per byte for a single page (--page RAM5, ROM0, ...),
    `--width` bytes per row (default 128, so 16k = 128x128 tiles). Each tile
    is `--tile-size` pixels (default 5) with a 1px top/left separator line
    only (no bottom/right dark edge, unlike the sectors bevel), so the
    solid-color area covers the rest of the tile instead of being squeezed
    from both sides. Colors blend the same way as gradient (green=read,
    red=write, cyan=exec) but intensity is quantized to `--levels` discrete
    steps (default 4) instead of a smooth gradient, so neighboring tiles
    read as clearly different "how hot" rather than blurring together.
    Untouched cells stay flat gray, same as sectors. Scaled to that page's
    own max, for maximum contrast within it.

  full - the whole address space, same page grouping/layout as sectors,
    but each block is a detail-style tile grid instead of a flat sector.
    Intensity is scaled to one shared max across the whole file (not
    per-page like --style detail), so hotness is comparable page to page.
    This is the "generate later" big picture from --style detail; sectors
    remains the quick categorical overview.
"""

import argparse
import csv
import math
import re
import struct
import sys
import zlib
from pathlib import Path

PAGE_SIZE = 16384
SECTOR_SIZE = 256
ROW_WIDTH = 256

# --- colors: tweak these to change the palette -----------------------------
COL_READ = (46, 207, 46)
COL_WRITE = (255, 57, 57)
COL_EXEC = (36, 159, 159)
COL_EMPTY = (232, 232, 232)
COL_WHITE = (248, 248, 248)
COL_BLACK = (0, 0, 0)
COL_BEVEL_LIGHT = (255, 255, 255)
COL_BEVEL_DARK = (208, 208, 208)
BORDER_SIZE = 16  # blank margin around the whole image, in final (post-scale) pixels
# -----------------------------------------------------------------------------

# Minimal 4x6 bitmap font, just the glyphs used for labels (hex digits + letters for RAM/ROM).
# Each glyph is 6 rows of a 4-bit mask (MSB = leftmost pixel).
FONT = {
    '0': [0x6, 0x9, 0x9, 0x9, 0x9, 0x6],
    '1': [0x2, 0x6, 0x2, 0x2, 0x2, 0x7],
    '2': [0x6, 0x9, 0x1, 0x2, 0x4, 0xf],
    '3': [0xf, 0x2, 0x6, 0x1, 0x9, 0x6],
    '4': [0x1, 0x3, 0x5, 0x9, 0xf, 0x1],
    '5': [0xf, 0x8, 0xe, 0x1, 0x9, 0x6],
    '6': [0x6, 0x8, 0xe, 0x9, 0x9, 0x6],
    '7': [0xf, 0x1, 0x2, 0x4, 0x4, 0x4],
    '8': [0x6, 0x9, 0x6, 0x9, 0x9, 0x6],
    '9': [0x6, 0x9, 0x9, 0x7, 0x1, 0x6],
    'A': [0x6, 0x9, 0x9, 0xf, 0x9, 0x9],
    'B': [0xe, 0x9, 0xe, 0x9, 0x9, 0xe],
    'C': [0x6, 0x9, 0x8, 0x8, 0x9, 0x6],
    'D': [0xe, 0x9, 0x9, 0x9, 0x9, 0xe],
    'E': [0xf, 0x8, 0xe, 0x8, 0x8, 0xf],
    'F': [0xf, 0x8, 0xe, 0x8, 0x8, 0x8],
    'M': [0x9, 0xf, 0xf, 0x9, 0x9, 0x9],
    'O': [0x6, 0x9, 0x9, 0x9, 0x9, 0x6],
    'P': [0xe, 0x9, 0x9, 0xe, 0x8, 0x8],
    'R': [0xe, 0x9, 0x9, 0xe, 0x9, 0x9],
    '#': [0x5, 0xf, 0x5, 0xf, 0x5, 0x0],
}
GLYPH_W, GLYPH_H = 4, 6
LABEL_MARGIN = 40  # left margin width in (unscaled) pixels, gradient style
PAGE_SPEC_RE = re.compile(r"^(RAM|ROM)(\d+)$")  # --page spec, e.g. RAM5, ROM0


class Canvas:
    def __init__(self, width, height, bg=COL_WHITE):
        self.width = width
        self.height = height
        self.stride = 1 + width * 3
        row = bytes(bg) * width
        self.raw = bytearray((b"\x00" + row) * height)

    def set_pixel(self, x, y, color):
        if 0 <= x < self.width and 0 <= y < self.height:
            off = y * self.stride + 1 + x * 3
            self.raw[off:off + 3] = bytes(color)

    def fill_rect(self, x0, y0, w, h, color):
        col = bytes(color)
        for y in range(max(0, y0), min(self.height, y0 + h)):
            off = y * self.stride + 1 + max(0, x0) * 3
            for x in range(max(0, x0), min(self.width, x0 + w)):
                self.raw[off:off + 3] = col
                off += 3

    def draw_text(self, x0, y0, text, color=COL_BLACK):
        for i, ch in enumerate(text):
            glyph = FONT.get(ch)
            if not glyph:
                continue
            gx = x0 + i * (GLYPH_W + 1)
            for gy, bits in enumerate(glyph):
                for bit in range(GLYPH_W):
                    if bits & (1 << (GLYPH_W - 1 - bit)):
                        self.set_pixel(gx + bit, y0 + gy, color)


def parse_csv_file(path):
    banks = {"RAM": {}, "ROM": {}}
    with open(path, "r", newline="") as f:
        for row in csv.DictReader(f):
            t = row["type"]
            if t not in banks:
                continue
            idx = int(row["page"]) * PAGE_SIZE + int(row["offset"])
            banks[t][idx] = (int(row["read"]), int(row["write"]), int(row["exec"]))
    return banks


def bank_page_count(cells):
    if not cells:
        return 0
    return (max(cells) // PAGE_SIZE) + 1


def page_touched(banks, t, page):
    base = page * PAGE_SIZE
    return any(any(v) for idx, v in banks[t].items() if base <= idx < base + PAGE_SIZE)


def extra_pages(banks, t, pages, exclude, mode):
    for page in range(pages):
        if page not in exclude and (mode == "128k" or page_touched(banks, t, page)):
            yield page


# --- style: sectors -------------------------------------------------------

def build_row_groups(ram_pages, rom_pages, banks, mode):
    """Ordered list of (row_base_address, [(block_label, type, page), ...]).

    Pages that share one CPU slot (ROM at 0000, RAM at C000) are grouped on
    the same row; RAM0 is only the *default* occupant of C000, not fixed
    there the way page 5 and page 2 are fixed at 4000/8000.
    """
    groups = []
    if rom_pages > 0:
        if mode != "48k":
            rom_blocks = [("ROM0", "ROM", 0)]
            rom_blocks += [(f"ROM{p}", "ROM", p) for p in extra_pages(banks, "ROM", rom_pages, {0}, mode)]
        else:
            rom_blocks = [("ROM0", "ROM", 1)]
        groups.append((0x0000, rom_blocks))
    if ram_pages > 5:
        groups.append((0x4000, [("RAM5", "RAM", 5)]))
    if ram_pages > 2:
        groups.append((0x8000, [("RAM2", "RAM", 2)]))
    if ram_pages > 0:
        ram_blocks = [("RAM0", "RAM", 0)]
        if mode != "48k":
            ram_blocks += [(f"RAM{p}", "RAM", p) for p in extra_pages(banks, "RAM", ram_pages, {0, 2, 5}, mode)]
        groups.append((0xC000, ram_blocks))
    return groups


def sector_flags(cells, base_idx):
    r = w = e = False
    for i in range(base_idx, base_idx + SECTOR_SIZE):
        nr, nw, ne = cells.get(i, (0, 0, 0))
        r = r or nr > 0
        w = w or nw > 0
        e = e or ne > 0
        if r and w and e:
            break
    return r, w, e


def draw_sector(canvas, x0, y0, size, r, w, e):
    active = [c for c, flag in ((COL_READ, r), (COL_WRITE, w), (COL_EXEC, e)) if flag]
    if not active:
        canvas.fill_rect(x0, y0, size, size, COL_EMPTY)
    elif len(active) == 1:
        canvas.fill_rect(x0, y0, size, size, active[0])
    else:
        # diagonal bands: distance along the top-left -> bottom-right diagonal
        # picks which of the active colors a pixel falls into
        n = len(active)
        span = 2 * (size - 1) + 1
        for dy in range(size):
            for dx in range(size):
                band = min(n - 1, (dx + dy) * n // span)
                canvas.set_pixel(x0 + dx, y0 + dy, active[band])
    draw_bevel(canvas, x0, y0, size)


def draw_bevel(canvas, x0, y0, size):
    """Windows-9x-style raised button edge: light top/left, dark bottom/right."""
    if size < 2:
        return
    for i in range(size):
        canvas.set_pixel(x0 + i, y0, COL_BEVEL_LIGHT)
        canvas.set_pixel(x0, y0 + i, COL_BEVEL_LIGHT)
        canvas.set_pixel(x0 + i, y0 + size - 1, COL_BEVEL_DARK)
        canvas.set_pixel(x0 + size - 1, y0 + i, COL_BEVEL_DARK)


def draw_bevel_topleft(canvas, x0, y0, size):
    """Top/left-only tile separator line: no bottom/right dark edge, so the
    color fill extends into that space instead of the bevel eating it."""
    if size < 2:
        return
    for i in range(size):
        canvas.set_pixel(x0 + i, y0, COL_BEVEL_LIGHT)
        canvas.set_pixel(x0, y0 + i, COL_BEVEL_LIGHT)


def render_sectors(banks, ram_pages, rom_pages, mode, grid_w, grid_h, tile, gap):
    if grid_w * grid_h != PAGE_SIZE // SECTOR_SIZE:
        print(f"--grid {grid_w}x{grid_h} must cover exactly {PAGE_SIZE // SECTOR_SIZE} sectors "
              f"(16384 bytes / {SECTOR_SIZE}-byte sectors)", file=sys.stderr)
        sys.exit(1)

    groups = build_row_groups(ram_pages, rom_pages, banks, mode)
    if not groups:
        print("nothing to render for this mode", file=sys.stderr)
        sys.exit(1)

    block_w = gap + grid_w * (tile + gap)
    block_h = gap + grid_h * (tile + gap)
    caption_h = GLYPH_H + 3
    block_gap = 6
    row_gap = 6

    row_widths = [len(blocks) * block_w + (len(blocks) - 1) * block_gap for _, blocks in groups]
    width = LABEL_MARGIN + max(row_widths)
    height = sum(caption_h + block_h for _ in groups) + row_gap * (len(groups) - 1)

    row_bytes = grid_w * SECTOR_SIZE  # bytes covered by one sector row, e.g. 8*256 = 0x800

    canvas = Canvas(width, height)
    y = 0
    for row_base, blocks in groups:
        for sy in range(grid_h):
            row_y = y + caption_h + gap + sy * (tile + gap)
            addr = row_base + sy * row_bytes
            canvas.draw_text(2, row_y + max(0, (tile - GLYPH_H) // 2), f"#{addr:04X}")
        x = LABEL_MARGIN
        for block_label, t, page in blocks:
            canvas.draw_text(x, y, block_label)
            cells = banks[t]
            base_page = page * PAGE_SIZE
            by = y + caption_h
            for sy in range(grid_h):
                for sx in range(grid_w):
                    sector_idx = base_page + (sy * grid_w + sx) * SECTOR_SIZE
                    r, w, e = sector_flags(cells, sector_idx)
                    tx = x + gap + sx * (tile + gap)
                    ty = by + gap + sy * (tile + gap)
                    draw_sector(canvas, tx, ty, tile, r, w, e)
            x += block_w + block_gap
        y += caption_h + block_h + row_gap

    return canvas, len(groups)


def blend_color(r_amt, w_amt, e_amt):
    """Blend read/write/exec intensities (each 0.0-1.0) into an RGB color.

    Shared by the gradient and detail styles so both use the same palette
    math; only how r_amt/w_amt/e_amt are derived (continuous vs quantized)
    differs between the two.
    """
    pr = 255 - round(255 * min(1.0, r_amt + e_amt))
    pg = 255 - round(255 * min(1.0, w_amt + e_amt))
    pb = 255 - round(255 * min(1.0, w_amt + r_amt))
    return (pr, pg, pb)


# --- style: gradient -------------------------------------------------------

def build_blocks(ram_pages, rom_pages, banks, mode):
    """Ordered list of (label, type, page) blocks to stack top to bottom."""
    blocks = []
    canonical = [("#0000", "ROM", 0), ("#4000", "RAM", 5), ("#8000", "RAM", 2), ("#C000", "RAM", 0)]
    for label, t, page in canonical:
        pages = rom_pages if t == "ROM" else ram_pages
        if page < pages:
            blocks.append((label, t, page))
    if mode == "48k":
        return blocks
    for page in extra_pages(banks, "RAM", ram_pages, {0, 2, 5}, mode):
        blocks.append((f"RAM{page}", "RAM", page))
    for page in extra_pages(banks, "ROM", rom_pages, {0}, mode):
        blocks.append((f"ROM{page}", "ROM", page))
    return blocks


def render_gradient(banks, ram_pages, rom_pages, mode, width, use_log):
    blocks = build_blocks(ram_pages, rom_pages, banks, mode)
    if not blocks:
        print("nothing to render for this mode", file=sys.stderr)
        sys.exit(1)
    rows_per_block = PAGE_SIZE // width

    max_r = max((v[0] for b in banks.values() for v in b.values()), default=0)
    max_w = max((v[1] for b in banks.values() for v in b.values()), default=0)
    max_e = max((v[2] for b in banks.values() for v in b.values()), default=0)

    def chan_scale(v, vmax):
        if v <= 0 or vmax <= 0:
            return 0.0
        if use_log:
            return math.log2(v + 1) / math.log2(vmax + 1)
        return v / vmax

    canvas = Canvas(LABEL_MARGIN + width, rows_per_block * len(blocks))
    for bi, (label, t, page) in enumerate(blocks):
        base_idx = page * PAGE_SIZE
        cells = banks[t]
        y_base = bi * rows_per_block
        canvas.draw_text(2, y_base + 1, label)
        for r in range(rows_per_block):
            y = y_base + r
            for x in range(width):
                idx = base_idx + r * width + x
                nr, nw, ne = cells.get(idx, (0, 0, 0))
                if nr == 0 and nw == 0 and ne == 0:
                    continue
                r_amt = chan_scale(nr, max_r)
                w_amt = chan_scale(nw, max_w)
                e_amt = chan_scale(ne, max_e)
                canvas.set_pixel(LABEL_MARGIN + x, y, blend_color(r_amt, w_amt, e_amt))

    return canvas, len(blocks)


# --- style: detail ----------------------------------------------------------

def parse_page_spec(spec, ram_pages, rom_pages):
    m = PAGE_SPEC_RE.match(spec.strip().upper())
    if not m:
        print(f"--page {spec!r} must look like RAM5 or ROM0", file=sys.stderr)
        sys.exit(1)
    t, page = m.group(1), int(m.group(2))
    pages = ram_pages if t == "RAM" else rom_pages
    if page >= pages:
        if pages == 0:
            print(f"--page {spec}: file has no {t} data", file=sys.stderr)
        else:
            print(f"--page {spec}: file only has {pages} {t} page(s) (0..{pages - 1})", file=sys.stderr)
        sys.exit(1)
    return t, page


def quantize_level(v, vmax, levels):
    """Bucket a hit count into 0..levels: 0 means untouched, 1..levels are
    log2-scaled intensity steps (so a single hit already shows as level 1,
    not lost in the same bucket as untouched cells)."""
    if v <= 0 or vmax <= 0:
        return 0
    scaled = math.log2(v + 1) / math.log2(vmax + 1)
    return max(1, min(levels, math.ceil(scaled * levels)))


def draw_detail_block(canvas, cells, base_idx, x0, y0, width, tile, levels, max_r, max_w, max_e, label=None):
    """Draw one page's worth of per-byte detail tiles at (x0, y0).

    Shared by render_detail (single page, scaled to its own max) and
    render_detail_map (whole address space, scaled to one shared max so
    hotness is comparable across pages).
    """
    caption_h = GLYPH_H + 3 if label else 0
    if label:
        canvas.draw_text(x0 + 2, y0, label)
    rows = PAGE_SIZE // width
    for row in range(rows):
        y = y0 + caption_h + row * tile
        for col in range(width):
            idx = base_idx + row * width + col
            nr, nw, ne = cells.get(idx, (0, 0, 0))
            x = x0 + col * tile
            if nr == 0 and nw == 0 and ne == 0:
                canvas.fill_rect(x, y, tile, tile, COL_EMPTY)
            else:
                r_lvl = quantize_level(nr, max_r, levels)
                w_lvl = quantize_level(nw, max_w, levels)
                e_lvl = quantize_level(ne, max_e, levels)
                color = blend_color(r_lvl / levels, w_lvl / levels, e_lvl / levels)
                canvas.fill_rect(x, y, tile, tile, color)
            draw_bevel_topleft(canvas, x, y, tile)


def render_detail(banks, page_type, page_num, width, tile, levels):
    cells = banks[page_type]
    base_idx = page_num * PAGE_SIZE
    rows = PAGE_SIZE // width

    max_r = max_w = max_e = 0
    for i in range(base_idx, base_idx + PAGE_SIZE):
        nr, nw, ne = cells.get(i, (0, 0, 0))
        max_r = max(max_r, nr)
        max_w = max(max_w, nw)
        max_e = max(max_e, ne)

    caption_h = GLYPH_H + 3
    canvas = Canvas(width * tile, caption_h + rows * tile)
    draw_detail_block(canvas, cells, base_idx, 0, 0, width, tile, levels, max_r, max_w, max_e,
                       label=f"{page_type}{page_num}")
    return canvas


def render_detail_map(banks, ram_pages, rom_pages, mode, width, tile, levels):
    """Full address-space map, like render_sectors, but each block is a
    per-byte detail grid (render_detail's tile style) instead of a flat
    per-256-byte sector. Intensity is scaled to one shared max across the
    whole file, so a page's hotness can be compared against every other
    page's, unlike the single-page render_detail (which scales to its own
    max for maximum contrast on just that page).
    """
    groups = build_row_groups(ram_pages, rom_pages, banks, mode)
    if not groups:
        print("nothing to render for this mode", file=sys.stderr)
        sys.exit(1)

    rows_per_block = PAGE_SIZE // width
    caption_h = GLYPH_H + 3
    block_w = width * tile
    block_h = caption_h + rows_per_block * tile
    block_gap = 6
    row_gap = 6

    row_widths = [len(blocks) * block_w + (len(blocks) - 1) * block_gap for _, blocks in groups]
    canvas = Canvas(max(row_widths), len(groups) * block_h + row_gap * (len(groups) - 1))

    max_r = max((v[0] for b in banks.values() for v in b.values()), default=0)
    max_w = max((v[1] for b in banks.values() for v in b.values()), default=0)
    max_e = max((v[2] for b in banks.values() for v in b.values()), default=0)

    y = 0
    for _, blocks in groups:
        x = 0
        for label, t, page in blocks:
            draw_detail_block(canvas, banks[t], page * PAGE_SIZE, x, y, width, tile, levels,
                               max_r, max_w, max_e, label=label)
            x += block_w + block_gap
        y += block_h + row_gap

    return canvas, len(groups)


# --- png output -------------------------------------------------------

def upscale(canvas, factor):
    if factor == 1:
        return canvas
    out = Canvas(canvas.width * factor, canvas.height * factor)
    for y in range(canvas.height):
        row_off = y * canvas.stride + 1
        row = canvas.raw[row_off: row_off + canvas.width * 3]
        wide_row = bytearray()
        for x in range(canvas.width):
            wide_row += row[x * 3:x * 3 + 3] * factor
        for dy in range(factor):
            oy = y * factor + dy
            ooff = oy * out.stride + 1
            out.raw[ooff: ooff + len(wide_row)] = wide_row
    return out


def add_border(canvas, px, color=COL_WHITE):
    if px <= 0:
        return canvas
    out = Canvas(canvas.width + 2 * px, canvas.height + 2 * px, bg=color)
    for y in range(canvas.height):
        row_off = y * canvas.stride + 1
        row = canvas.raw[row_off: row_off + canvas.width * 3]
        oy = y + px
        ooff = oy * out.stride + 1 + px * 3
        out.raw[ooff: ooff + len(row)] = row
    return out


def write_png(path, canvas):
    def chunk(tag, data):
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xffffffff)

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", canvas.width, canvas.height, 8, 2, 0, 0, 0)  # 8-bit RGB, no interlace
    idat = zlib.compress(bytes(canvas.raw), 9)
    with open(path, "wb") as f:
        f.write(sig)
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", idat))
        f.write(chunk(b"IEND", b""))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv_file", help="path to a .heat.csv file exported from xpeccy")
    ap.add_argument("-o", "--output", help="output PNG path (default: input path with .png)")
    ap.add_argument("--style", choices=("sectors", "gradient", "detail", "full"), default="sectors",
                     help="sectors (default): flat per-256-byte-sector categorical map; "
                          "gradient: per-byte log-scaled blend; "
                          "detail: per-byte tiles for a single page (see --page); "
                          "full: per-byte tiles for the whole address space, sectors' layout")
    ap.add_argument("--mode", choices=("auto", "48k", "128k"), default="auto",
                     help="auto (default): fixed slots (0000/4000/8000/C000) plus any other page that "
                          "was actually touched; 48k: only the fixed slots, ignore every switched page "
                          "even if it has stale data; 128k: every page in the file, touched or not "
                          "(ignored by --style detail, which always shows the one --page requested)")
    ap.add_argument("--scale", type=int, default=None, choices=(1, 2, 3, 4),
                     help="nearest-neighbor upscale factor (1-4, default 2; 1 for detail/full)")
    ap.add_argument("--grid", default="8x8", help="sectors style: sector grid per 16k page, WxH, must multiply to 64 (default 8x8)")
    ap.add_argument("--tile-size", type=int, default=None,
                     help="sectors/detail/full style: tile size in pixels (default 12 for sectors, 5 for detail/full)")
    ap.add_argument("--gap", type=int, default=0, help="sectors style: gap between sector tiles in pixels (default 0)")
    ap.add_argument("--width", type=int, default=None,
                     help="gradient/detail/full style: bytes per row / pixels of data width "
                          "(default 256 for gradient, 128 for detail/full)")
    ap.add_argument("--linear", action="store_true", help="gradient style: linear scaling instead of log2 (hot cells wash out everything else)")
    ap.add_argument("--page", help="detail style: which page to render, e.g. RAM5, ROM0")
    ap.add_argument("--levels", type=int, default=4, help="detail/full style: number of log2-scaled intensity steps per channel (default 4)")
    args = ap.parse_args()

    banks = parse_csv_file(args.csv_file)
    ram_pages = bank_page_count(banks["RAM"])
    rom_pages = bank_page_count(banks["ROM"])
    if ram_pages == 0 and rom_pages == 0:
        print("no RAM or ROM data found in the file", file=sys.stderr)
        sys.exit(1)

    tile_detail_default = 5
    if args.style == "sectors":
        tile_size = args.tile_size if args.tile_size is not None else 12
        gw, gh = (int(v) for v in args.grid.lower().split("x"))
        canvas, n = render_sectors(banks, ram_pages, rom_pages, args.mode, gw, gh, tile_size, args.gap)
    elif args.style == "gradient":
        width = args.width if args.width is not None else ROW_WIDTH
        canvas, n = render_gradient(banks, ram_pages, rom_pages, args.mode, width, not args.linear)
    elif args.style == "detail":
        if not args.page:
            print("--style detail requires --page (e.g. --page RAM5)", file=sys.stderr)
            sys.exit(1)
        tile_size = args.tile_size if args.tile_size is not None else tile_detail_default
        width = args.width if args.width is not None else 128
        page_type, page_num = parse_page_spec(args.page, ram_pages, rom_pages)
        canvas = render_detail(banks, page_type, page_num, width, tile_size, args.levels)
        n = 1
    else:  # full
        tile_size = args.tile_size if args.tile_size is not None else tile_detail_default
        width = args.width if args.width is not None else 128
        canvas, n = render_detail_map(banks, ram_pages, rom_pages, args.mode, width, tile_size, args.levels)

    scale = args.scale if args.scale is not None else (1 if args.style in ("detail", "full") else 2)
    canvas = upscale(canvas, scale)
    canvas = add_border(canvas, BORDER_SIZE)
    out_path = args.output or str(Path(args.csv_file).with_suffix("").with_suffix(".png"))
    write_png(out_path, canvas)
    print(f"{out_path}: {n} groups/blocks, {canvas.width}x{canvas.height}")


if __name__ == "__main__":
    main()
