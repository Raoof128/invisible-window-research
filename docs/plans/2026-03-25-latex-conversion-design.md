# LaTeX Conversion Design — The Invisible Window

**Date:** 2026-03-25
**Target:** IEEE Conference (`\documentclass[conference]{IEEEtran}`)
**Primary venue:** IEEE EDUCON 2026
**Fallback venue:** IEEE EuroS&P 2026

## File Structure

```
paper/
├── main.tex                    # Full paper
├── references.bib              # 51 BibTeX entries
├── figures/
│   ├── attack-architecture.tex # TikZ diagram (Fig. 1)
│   ├── windows-diff.png        # Forensic diff (Fig. 2)
│   └── macos-capture.png       # macOS composite (Fig. 3)
└── Makefile                    # latexmk build
```

## Document Setup

- `\documentclass[conference]{IEEEtran}`
- Packages: graphicx, xcolor, listings, tikz, hyperref, amsmath, amssymb, cite
- Author block: `\IEEEauthorblockN` / `\IEEEauthorblockA`
- AI methodology in `\thanks{}` footnote on title
- Active abstract: AI Safety version
- Original abstract preserved as LaTeX comment

## Figures

1. **Fig. 1 — Attack Architecture** (TikZ, `figure*` full-width): Three-layer diagram showing Physical Display → OS Compositing → Capture API trust boundary violation
2. **Fig. 2 — Windows Forensic Diff** (`figure`, single-column): `round1_DIFF_AB.png` showing 80.27% pixel diff
3. **Fig. 3 — macOS Capture Comparison** (`figure`, single-column): Side-by-side composite of transparent single-window capture vs visible content

## Tables

- **Table I**: Screen Capture Evasion results (Section V-C)
- **Table II**: Countermeasure landscape (Section VI-F)
- All other inline tables merged into prose or these two tables

## Page Budget

IEEE conference limit: 8 pages + refs. Target: ~8,200 words (down from 9,500).
Trim strategy:
- Section VIII (Related Work): condense each subsection to 2-3 sentences (~400 words saved)
- Section VII (Ethics): condense VII-A/B/C into one paragraph (~200 words saved)
- Minor tightening across Sections II-IV

## Build

```makefile
all:
	latexmk -pdf -interaction=nonstopmode main.tex
clean:
	latexmk -C
```

## Pre-requisites Before Conversion

1. Trim markdown paper to ~8,200 words
2. Generate Fig. 3 macOS composite image
