#!/usr/bin/env python3
"""Generate Figure 3: macOS capture comparison composite.

Left panel  — what CGWindowListCreateImage returns (solid black / fully transparent)
Right panel — what the user actually sees on the physical display
"""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

# ---------------------------------------------------------------------------
# Canvas
# ---------------------------------------------------------------------------
WIDTH, HEIGHT = 900, 400
PANEL_W, PANEL_H = 420, 310
GAP = 10
TOP_MARGIN = 50                       # room for panel labels
PANEL_Y = TOP_MARGIN + 10
LEFT_X = (WIDTH - 2 * PANEL_W - GAP) // 2
RIGHT_X = LEFT_X + PANEL_W + GAP

# ---------------------------------------------------------------------------
# Fonts — fall back gracefully
# ---------------------------------------------------------------------------
def _font(size: int, bold: bool = False, mono: bool = False) -> ImageFont.FreeTypeFont:
    candidates: list[str] = []
    if mono:
        candidates += [
            "/System/Library/Fonts/SFMono-Regular.otf",
            "/System/Library/Fonts/Menlo.ttc",
            "/System/Library/Fonts/Courier.dfont",
        ]
    elif bold:
        candidates += [
            "/System/Library/Fonts/SFNSTextCondensed-Bold.otf",
            "/System/Library/Fonts/Helvetica.ttc",
        ]
    else:
        candidates += [
            "/System/Library/Fonts/SFNSText.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
        ]
    for path in candidates:
        try:
            return ImageFont.truetype(path, size)
        except (OSError, IOError):
            continue
    return ImageFont.load_default()


label_font   = _font(13, bold=True)
small_font   = _font(11)
overlay_font = _font(16, bold=True)
mono_font    = _font(11, mono=True)
title_font   = _font(10)
bottom_font  = _font(11)

# ---------------------------------------------------------------------------
# Canvas setup
# ---------------------------------------------------------------------------
img = Image.new("RGB", (WIDTH, HEIGHT), "white")
draw = ImageDraw.Draw(img)

# ---------------------------------------------------------------------------
# Panel labels (centred above each panel)
# ---------------------------------------------------------------------------
left_label = "Single-Window Capture (CGWindowListCreateImage)"
right_label = "Visible on Physical Display"

def draw_centred(text: str, cx: float, y: float, font, fill="black"):
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    draw.text((cx - tw / 2, y), text, font=font, fill=fill)

draw_centred(left_label,  LEFT_X + PANEL_W / 2,  TOP_MARGIN - 22, label_font)
draw_centred(right_label, RIGHT_X + PANEL_W / 2, TOP_MARGIN - 22, label_font)

# ---------------------------------------------------------------------------
# LEFT PANEL — solid black (representing RGBA 0,0,0,0 capture output)
# ---------------------------------------------------------------------------
draw.rectangle([LEFT_X, PANEL_Y, LEFT_X + PANEL_W, PANEL_Y + PANEL_H], fill="black")

# White overlay text
draw_centred("100% transparent",
             LEFT_X + PANEL_W / 2, PANEL_Y + PANEL_H / 2 - 30, overlay_font, fill="white")
draw_centred("1,170,560 pixels  RGBA(0, 0, 0, 0)",
             LEFT_X + PANEL_W / 2, PANEL_Y + PANEL_H / 2 + 5, small_font, fill="#999999")

# ---------------------------------------------------------------------------
# RIGHT PANEL — simulated "Invisible Window" as the user sees it
# ---------------------------------------------------------------------------
# Red border (3 px)
BORDER = 3
draw.rectangle(
    [RIGHT_X, PANEL_Y, RIGHT_X + PANEL_W, PANEL_Y + PANEL_H],
    outline="#ff3b30", width=BORDER,
)

# Dark background inside the border
inner = [RIGHT_X + BORDER, PANEL_Y + BORDER,
         RIGHT_X + PANEL_W - BORDER, PANEL_Y + PANEL_H - BORDER]
draw.rectangle(inner, fill="#1a1a26")

# Title bar
TB_H = 24
tb_rect = [inner[0], inner[1], inner[2], inner[1] + TB_H]
draw.rectangle(tb_rect, fill="#2a2a3a")
draw_centred("Invisible Window (sharingType = .none)",
             (inner[0] + inner[2]) / 2, inner[1] + 4, title_font, fill="#aaaacc")

# Simulated code / notes in bright green monospace
code_lines = [
    "// Research notes — The Invisible Window",
    "",
    "let window = NSWindow(",
    '    contentRect: NSRect(x:0, y:0, w:1124, h:1042),',
    '    styleMask: [.titled, .resizable],',
    '    backing: .buffered, defer: false',
    ")",
    "window.sharingType = .none",
    "window.title = \"Invisible Window\"",
    "",
    "// This window is VISIBLE on display",
    "// but INVISIBLE to CGWindowListCreateImage",
]
y = inner[1] + TB_H + 8
for line in code_lines:
    draw.text((inner[0] + 12, y), line, font=mono_font, fill="#39ff14")
    y += 16

# ---------------------------------------------------------------------------
# Bottom label
# ---------------------------------------------------------------------------
draw_centred("macOS 26.3.1 \u2014 sharingType = .none",
             WIDTH / 2, HEIGHT - 24, bottom_font, fill="#555555")

# ---------------------------------------------------------------------------
# Save
# ---------------------------------------------------------------------------
out_dir = Path(__file__).resolve().parent / "figures"
out_dir.mkdir(parents=True, exist_ok=True)
out_path = out_dir / "macos-capture.png"
img.save(out_path, dpi=(300, 300))
print(f"Saved {out_path}  ({img.size[0]}x{img.size[1]})")
