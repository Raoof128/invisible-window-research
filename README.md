# The Invisible Window

**Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems**

**Author:** Mohammad Raouf Abedini ([ORCID](https://orcid.org/0009-0000-6214-258X))
**Affiliation:** Department of Computing, Macquarie University, Sydney, Australia
**Contact:** mohammadraouf.abedini@students.mq.edu.au | [raoufabedini.dev](https://raoufabedini.dev)

---

## Overview

Browser-based proctoring systems trust that `getDisplayMedia()` captures what the user sees. Documented OS APIs (`SetWindowDisplayAffinity` on Windows, `NSWindow.SharingType.none` on macOS) break that assumption: windows can be visible on the physical display while invisible to all screen capture. Commercial tools (Cluely, Interview Coder) already exploit this to embed AI assistants as invisible overlays.

This repository contains the paper, proof-of-concept implementations, and forensic evaluation data.

## Repository Structure

```
paper/                          # arXiv-ready LaTeX paper (12 pages, 51 citations)
  main.tex                      # Source
  main.bbl                      # Compiled bibliography
  references.bib                # BibTeX source
  figures/                      # Forensic diff images
  Makefile                      # Build script

poc/                            # Proof-of-concept implementations
  windows/                      # Win32 C (WDA_EXCLUDEFROMCAPTURE)
  macos/                        # Swift (sharingType = .none)
  linux/                        # X11 analysis (NOT vulnerable)

docs/                           # Supporting documents
  invisible-window-paper.md     # Markdown draft
  invisible-window-references.md
  ARXIV-SUBMISSION-GUIDE.md     # arXiv submission checklist
  emails-to-anthropic.md        # Anthropic program submissions

reasoning-engine/               # ACPR reasoning MCP server
```

## Key Results

| Platform | Evasion Rate | Artefacts |
|----------|-------------|-----------|
| Windows 10/11 | 100% | None |
| macOS 14 (Sonoma) | 100% | None |
| macOS 26.3.1 | 100% | None |
| Linux (X11/Wayland) | N/A | Not vulnerable |

macOS 26 remains fully vulnerable despite Apple's documented ScreenCaptureKit changes in macOS 15.

## Research Methodology

Conducted using Claude Code powered by Claude Opus 4.6 (1M context). The AI-assisted methodology and dual-use implications are documented in Section VIII-G of the paper.

## Responsible Disclosure

Proctoring vendors notified January 2026. OS vendors notified February 2026. Paper published March 2026 after the 90-day disclosure window.

## License

CC BY 4.0 (arXiv preprint). See the paper for full terms.
