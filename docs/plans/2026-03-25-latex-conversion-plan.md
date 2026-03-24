# LaTeX Conversion Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Convert the Invisible Window research paper from Markdown to IEEE Conference LaTeX, ready for submission to IEEE EDUCON 2026.

**Architecture:** Single `main.tex` with BibTeX references, 3 figures (1 TikZ diagram, 2 raster images), targeting `\documentclass[conference]{IEEEtran}`. Two pre-requisites (trim word count, generate Fig. 3 composite) before the main conversion.

**Tech Stack:** LaTeX (IEEEtran), BibTeX, TikZ, latexmk, Python (PIL for Fig. 3 generation)

---

### Task 1: Create paper/ directory and scaffolding

**Files:**
- Create: `paper/Makefile`
- Create: `paper/figures/` (directory)

**Step 1: Create directory structure**

```bash
mkdir -p paper/figures
```

**Step 2: Create Makefile**

Create `paper/Makefile`:

```makefile
all:
	cd .. && latexmk -pdf -outdir=paper -interaction=nonstopmode paper/main.tex

clean:
	cd .. && latexmk -C -outdir=paper paper/main.tex

.PHONY: all clean
```

**Step 3: Copy Windows diff image**

```bash
cp poc/windows/test_results/round1_DIFF_AB.png paper/figures/windows-diff.png
```

**Step 4: Commit**

```bash
git add paper/Makefile paper/figures/windows-diff.png
git commit -m "scaffold: create paper/ directory with Makefile and Windows diff figure"
```

---

### Task 2: Generate Fig. 3 — macOS capture composite

**Files:**
- Create: `paper/generate_fig3.py`
- Output: `paper/figures/macos-capture.png`

**Step 1: Write the generation script**

Create `paper/generate_fig3.py`. This script creates a side-by-side composite showing:
- Left panel: "Single-Window Capture" — solid black rectangle (representing 100% transparent/black pixels returned by CGWindowListCreateImage)
- Right panel: "Visible on Display" — the actual window content (green text on dark background with red border, matching the PoC's appearance)
- Labels and captions explaining the evasion

Use PIL/Pillow to generate this programmatically. The image should be ~800x400px, suitable for single-column IEEE figure.

Key content:
- Left: black fill with white text "CGWindowListCreateImage output: 100% transparent (1,170,560 pixels RGBA 0,0,0,0)"
- Right: dark background (#1a1a26) with green monospace text simulating the PoC window, red border
- Bottom label: "sharingType = .none on macOS 26.3.1"

**Step 2: Run the script**

```bash
cd paper && python3 generate_fig3.py
```

Expected: `paper/figures/macos-capture.png` created.

**Step 3: Verify the image looks correct**

Open and visually inspect `paper/figures/macos-capture.png`.

**Step 4: Commit**

```bash
git add paper/generate_fig3.py paper/figures/macos-capture.png
git commit -m "fig: generate macOS capture comparison composite (Fig. 3)"
```

---

### Task 3: Trim markdown paper to ~8,200 words

**Files:**
- Modify: `docs/invisible-window-paper.md`

**Target:** Cut ~1,300 words to fit IEEE 8-page + refs limit.

**Step 1: Trim Section VIII (Related Work) — save ~500 words**

Condense each of VIII-A through VIII-F to 2-3 sentences each. Keep VIII-G (AI Amplification) at full length — it's the Anthropic pitch. The related work currently runs ~1,800 words; target ~1,300.

Specific cuts:
- VIII-A: Keep Simko [16] and Luijben [17] mentions, drop detailed description of Balash [3,7]
- VIII-B: Merge into 3 sentences covering the academic consensus
- VIII-C: Keep one sentence on gaze/mouse detection + the "our evaluation shows ineffective" conclusion
- VIII-D: Condense to 2 sentences
- VIII-E: Condense to 2 sentences
- VIII-F: Keep the commercial exploitation detail — it's novel and important

**Step 2: Trim Section VII (Ethics) — save ~300 words**

- VII-A (Disclosure): Keep the 4-step timeline, condense prose around it
- VII-B (Ethical Framing): Merge into VII-A as 2 sentences
- VII-C (Dual-Use): Keep points 1-4 but tighten each to one sentence
- VII-D (Impact): Condense to 2 sentences

**Step 3: Tighten Sections II-IV — save ~500 words**

- II-A: Trim WebRTC API description (reviewers know this)
- II-C: Compress the 5-layer architecture to a tighter list
- II-D: Condense security requirements (keep the numbered list, trim prose)
- III-A: Tighten actor descriptions
- IV-D: Trim operational considerations

**Step 4: Verify word count**

```bash
wc -w docs/invisible-window-paper.md
```

Target: ~8,200 words (down from 9,527).

**Step 5: Commit**

```bash
git add docs/invisible-window-paper.md
git commit -m "trim: reduce paper to ~8,200 words for IEEE 8-page limit"
```

---

### Task 4: Create references.bib

**Files:**
- Create: `paper/references.bib`

**Step 1: Convert all 51 references to BibTeX**

Read the References section of `docs/invisible-window-paper.md` (lines 523-625). Convert each `[N]` entry to a BibTeX entry. Use these key conventions:

- `@inproceedings` for conference papers
- `@article` for journal papers
- `@misc` for websites, GitHub repos, online resources
- `@techreport` for standards documents (W3C, IEEE codes, etc.)

Key format: `lastname_year_keyword` (e.g., `bruaroey2025screen`, `simko2024modern`, `microsoft2025setwindow`)

Ensure all 51 entries are present. Cross-check each entry against the markdown source.

**Step 2: Verify entry count**

```bash
grep -c '@' paper/references.bib
```

Expected: 51 entries.

**Step 3: Commit**

```bash
git add paper/references.bib
git commit -m "bib: add all 51 BibTeX references"
```

---

### Task 5: Write main.tex — preamble, abstract, Sections I-III

**Files:**
- Create: `paper/main.tex`

**Step 1: Write the preamble and front matter**

```latex
\documentclass[conference]{IEEEtran}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{graphicx}
\usepackage{xcolor}
\usepackage{listings}
\usepackage{tikz}
\usepackage{hyperref}
\usepackage{amsmath,amssymb}
\usepackage{cite}
\usepackage{booktabs}
\usepackage{url}

\usetikzlibrary{shapes,arrows.meta,positioning,fit,backgrounds}

\lstset{
  basicstyle=\ttfamily\footnotesize,
  breaklines=true,
  frame=single,
  language=C,
  numbers=left,
  numberstyle=\tiny\color{gray},
  keywordstyle=\color{blue},
  commentstyle=\color{green!50!black},
  stringstyle=\color{red}
}

\begin{document}

\title{The Invisible Window: Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems%
\thanks{Research conducted using Claude Code powered by Claude Opus 4.6. PoC implementations empirically validated on Windows 10 Build 19045 and macOS 26.3.1.}}

\author{\IEEEauthorblockN{Mohammad Raouf Abedini}
\IEEEauthorblockA{Department of Computing\\
Macquarie University, Sydney, Australia\\
mohammadraouf.abedini@students.mq.edu.au}}

\maketitle
```

Then convert the AI Safety abstract using `\begin{abstract}...\end{abstract}` and `\begin{IEEEkeywords}...\end{IEEEkeywords}`.

**Step 2: Convert Sections I-III from the trimmed markdown**

Source: `docs/invisible-window-paper.md` lines 22-185 (post-trim).

Convert:
- Section headings: `## I.` → `\section{}`
- Subsection headings: `### A.` → `\subsection{}`
- Bold text: `**text**` → `\textbf{text}`
- Italic text: `*text*` → `\textit{text}`
- Inline code: `` `code` `` → `\texttt{code}`
- Code blocks → `\begin{lstlisting}...\end{lstlisting}`
- Citations: `[N]` → `\cite{bibkey}` (use keys from references.bib)
- Definition/Theorem blocks → `\begin{quote}\textbf{Definition}...\end{quote}`
- Numbered lists → `\begin{enumerate}...\end{enumerate}`
- Bullet lists → `\begin{itemize}...\end{itemize}`

**Step 3: Test compile**

```bash
cd paper && latexmk -pdf -interaction=nonstopmode main.tex
```

Expected: PDF compiles (references will show as `[?]` until bibtex runs, that's fine).

**Step 4: Commit**

```bash
git add paper/main.tex
git commit -m "latex: preamble, abstract, Sections I-III"
```

---

### Task 6: Write main.tex — Section IV (Attack Design) with code listings

**Files:**
- Modify: `paper/main.tex`

**Step 1: Convert Section IV**

Source: `docs/invisible-window-paper.md` Section IV.

Key elements:
- C code listing for Windows implementation → `\begin{lstlisting}[language=C, caption={...}]`
- Swift code listing for macOS implementation → `\begin{lstlisting}[language=Swift, caption={...}]`
- The three attack variants (A, B, C) as a description list

**Step 2: Test compile**

```bash
cd paper && latexmk -pdf main.tex
```

**Step 3: Commit**

```bash
git add paper/main.tex
git commit -m "latex: Section IV (Attack Design) with code listings"
```

---

### Task 7: Write main.tex — Section V (Evaluation) with Tables I and Figures 2-3

**Files:**
- Modify: `paper/main.tex`

**Step 1: Convert Section V**

Source: `docs/invisible-window-paper.md` Section V.

Key elements:
- **Table I** — Screen Capture Evasion results:
  ```latex
  \begin{table}[t]
  \caption{Screen Capture Evasion Results}
  \label{tab:evasion}
  \centering
  \begin{tabular}{llcc}
  \toprule
  Platform & Capture Method & Evasion & Artefacts \\
  \midrule
  ...
  \bottomrule
  \end{tabular}
  \end{table}
  ```

- **Figure 2** — Windows diff:
  ```latex
  \begin{figure}[t]
  \centering
  \includegraphics[width=\columnwidth]{figures/windows-diff.png}
  \caption{Pixel diff between excluded and visible captures...}
  \label{fig:windows-diff}
  \end{figure}
  ```

- **Figure 3** — macOS composite:
  ```latex
  \begin{figure}[t]
  \centering
  \includegraphics[width=\columnwidth]{figures/macos-capture.png}
  \caption{macOS 26.3.1 single-window capture returns 100\% transparent pixels...}
  \label{fig:macos-capture}
  \end{figure}
  ```

- Behavioural detection results with p-values
- Process detection findings

**Step 2: Test compile**

```bash
cd paper && latexmk -pdf main.tex
```

Verify figures render correctly in the PDF.

**Step 3: Commit**

```bash
git add paper/main.tex
git commit -m "latex: Section V (Evaluation) with Table I and Figures 2-3"
```

---

### Task 8: Write main.tex — Section VI (Countermeasures) with Table II

**Files:**
- Modify: `paper/main.tex`

**Step 1: Convert Section VI**

Source: `docs/invisible-window-paper.md` Section VI.

Key element — **Table II** (Countermeasure landscape):
```latex
\begin{table}[t]
\caption{Countermeasure Assessment}
\label{tab:countermeasures}
\centering
\begin{tabular}{lccc}
\toprule
Countermeasure & Effectiveness & Deployability & Evasion Diff. \\
\midrule
...
\bottomrule
\end{tabular}
\end{table}
```

**Step 2: Test compile and commit**

```bash
cd paper && latexmk -pdf main.tex
git add paper/main.tex
git commit -m "latex: Section VI (Countermeasures) with Table II"
```

---

### Task 9: Write main.tex — Sections VII-VIII (Ethics, Related Work, AI Amplification)

**Files:**
- Modify: `paper/main.tex`

**Step 1: Convert Sections VII and VIII**

Source: `docs/invisible-window-paper.md` Sections VII-VIII (trimmed versions).

Section VIII-G (AI Amplification Analysis) should be converted carefully — it's the Anthropic pitch section. Preserve all bold subheadings as `\textbf{}` paragraphs.

**Step 2: Test compile and commit**

```bash
cd paper && latexmk -pdf main.tex
git add paper/main.tex
git commit -m "latex: Sections VII-VIII (Ethics, Related Work, AI Amplification)"
```

---

### Task 10: Write main.tex — Section IX (Conclusion) and Fig. 1 (TikZ architecture)

**Files:**
- Modify: `paper/main.tex`
- Create: `paper/figures/attack-architecture.tex`

**Step 1: Convert Section IX (Conclusion)**

Source: `docs/invisible-window-paper.md` Section IX.

**Step 2: Create TikZ architecture diagram**

Create `paper/figures/attack-architecture.tex` with a TikZ diagram showing:

```
+---------------------------+
| Physical Display          |
| +-------+ +------------+ |
| | Exam  | | Invisible  | |
| | Page  | | Window     | |
| +-------+ +------------+ |
+---------------------------+
         |
    OS Compositing Pipeline
    (respects display affinity)
         |
    ═══════════════════════  ← Trust boundary violation
         |
+---------------------------+
| Capture API Output        |
| +-------+                 |
| | Exam  |  [no window]    |
| | Page  |                 |
| +-------+                 |
+---------------------------+
         |
    getDisplayMedia()
         |
+---------------------------+
| Proctoring System         |
| "Screen looks clean"      |
+---------------------------+
```

Use `\tikzstyle`, `\node`, `\draw` with arrows. Colour the invisible window green, the trust boundary red dashed. Place as `figure*` (full-width, spanning both columns) after Section III or at top of Section IV.

Input it in main.tex:
```latex
\begin{figure*}[t]
\centering
\input{figures/attack-architecture.tex}
\caption{The Invisible Window attack architecture...}
\label{fig:architecture}
\end{figure*}
```

**Step 3: Add bibliography**

```latex
\bibliographystyle{IEEEtran}
\bibliography{references}

\end{document}
```

**Step 4: Full compile with BibTeX**

```bash
cd paper && latexmk -pdf main.tex
```

This runs pdflatex + bibtex + pdflatex + pdflatex. All `[?]` citations should resolve.

**Step 5: Commit**

```bash
git add paper/main.tex paper/figures/attack-architecture.tex
git commit -m "latex: Section IX, TikZ architecture diagram (Fig. 1), bibliography"
```

---

### Task 11: Final verification and page count check

**Files:**
- Modify: `paper/main.tex` (if trimming needed)

**Step 1: Check page count**

```bash
cd paper && pdfinfo main.pdf | grep Pages
```

Target: 8 pages of content + 1-2 pages of references = 9-10 total pages.

**Step 2: If over 8 content pages, trim further**

Tighten prose in the longest sections. Check for widows/orphans. Adjust figure placement with `[t]`, `[b]`, `[h]` hints.

**Step 3: Check for LaTeX warnings**

```bash
grep -i "warning\|underfull\|overfull" paper/main.log | head -20
```

Fix any overfull hbox warnings (usually from long URLs or code — wrap with `\url{}` or adjust listings).

**Step 4: Visual review**

Open `paper/main.pdf` and check:
- All 3 figures render correctly
- All tables are formatted properly
- All 51 citations resolve (no `[?]`)
- No orphaned text or broken formatting
- Code listings don't overflow column width

**Step 5: Final commit**

```bash
git add -A paper/
git commit -m "paper: finalise IEEE LaTeX submission (8 pages + refs)"
```

---

## Execution Notes

- LaTeX toolchain confirmed: `latexmk`, `pdflatex`, `IEEEtran.cls` all installed at `/Library/TeX/texbin/`
- All figure source files exist in the repo
- The trimming (Task 3) modifies the markdown source — the LaTeX conversion (Tasks 5-10) reads from the trimmed markdown
- Tasks 5-10 are sequential (each appends to main.tex)
- Task 4 (references.bib) is independent and can run in parallel with Task 3
