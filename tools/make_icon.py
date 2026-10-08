#!/usr/bin/env python3
"""Draw our original 25 px Chi-Rho launcher glyph, without a font dependency.

Requires Pillow. The SVG master and PNG use the same explicit stroke geometry.
The black transparent silhouette lets the Pebble launcher apply its own tint.
"""
from pathlib import Path

from PIL import Image, ImageDraw

DEST = Path(__file__).resolve().parents[1] / "resources/images"
DEST.mkdir(parents=True, exist_ok=True)
STROKES = [
    [(11, 23), (11, 2), (16, 2), (20, 4), (21, 7), (19, 10), (16, 11), (11, 11)],
    [(4, 9), (20, 22)],
    [(20, 9), (4, 22)],
]

image = Image.new("RGBA", (25, 25), (0, 0, 0, 0))
draw = ImageDraw.Draw(image)
for points in STROKES:
    draw.line(points, fill=(0, 0, 0, 255), width=2, joint="curve")
image.save(DEST / "chi-rho.png", optimize=True)

paths = []
for points in STROKES:
    coords = " L ".join(f"{x} {y}" for x, y in points)
    paths.append(f'  <path d="M {coords}"/>')
(DEST / "chi-rho.svg").write_text(
    '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 25 25">\n'
    ' <title>Chi-Rho — Daily Roman Missal launcher icon</title>\n'
    ' <g fill="none" stroke="black" stroke-width="2" stroke-linejoin="round">\n'
    + "\n".join(paths) + "\n </g>\n</svg>\n"
)
print(f"Created 25x25 transparent Chi-Rho in {DEST}")
