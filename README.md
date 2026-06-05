# The Invisible Window

[![DOI](https://img.shields.io/badge/DOI-10.5281%2Fzenodo.20376495-blue)](https://doi.org/10.5281/zenodo.20376495) [![DOI](https://img.shields.io/badge/DOI-10.5281%2Fzenodo.20195135-blue)](https://doi.org/10.5281/zenodo.20195135)

**Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems**

**Author:** Mohammad Raouf Abedini ([ORCID](https://orcid.org/0009-0000-6214-258X))  
**Affiliation:** Department of Computing, Macquarie University, Sydney, Australia  
**Contact:** mohammadraouf.abedini@students.mq.edu.au | [raoufabedini.dev](https://raoufabedini.dev)  
**Preprint:** https://doi.org/10.5281/zenodo.20376495

---

## Abstract

Browser-based proctoring systems rely on `getDisplayMedia()` under the implicit assumption that the captured frame faithfully represents the physical display. We demonstrate that this assumption is violated by documented OS-level display affinity APIs — `SetWindowDisplayAffinity` (`WDA_EXCLUDEFROMCAPTURE`) on Windows and `NSWindow.SharingType.none` on macOS — which allow application windows to be fully visible on the physical monitor while producing zero pixels in any screen capture output. We term this class of attack the *Invisible Window* and demonstrate 100% evasion on all tested platforms, including macOS 26 where the attack was previously assumed mitigated. Commercial tools (Cluely, Interview Coder) already exploit this to embed AI assistants as invisible overlays. We classify this as a **security-relevant downstream design vulnerability in capture-dependent systems** — not an OS zero-day — and propose countermeasures accordingly.

---

## Repository Structure

```
paper/                          # IEEE-format LaTeX paper (13 pages, 53 citations)
  main.tex                      # LaTeX source
  main.bbl                      # Compiled bibliography (arXiv-ready)
  references.bib                # BibTeX source
  main.pdf                      # Compiled paper
  figures/                      # Forensic diff images (Windows + macOS)
  Makefile                      # Build: make

poc/                            # Proof-of-concept implementations
  windows/                      # Win32 C — SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)
  macos/                        # Swift — NSWindow.sharingType = .none
  linux/                        # X11 analysis (not vulnerable)

docs/                           # Supporting documents
  invisible-window-paper.md     # Markdown draft
  ARXIV-SUBMISSION-GUIDE.md     # arXiv submission checklist

reasoning-engine/               # ACPR deep-research MCP server used during research
```

### Vendor Response Evidence (repo root)

| File | Contents |
|------|----------|
| `MIcrosoft- RE_ 111448 CRM_0034000320.pdf` | MSRC formal response (2 Apr 2026): classified as by-design, not a security vulnerability |
| `NSWindow.sharingType = .none hides visible windows from ScreenCaptureKit_WebRTC capture - My Reports - Apple Security Research.pdf` | Apple Product Security portal (reported 26 Mar, responded 28 Mar 2026): classified as consistent with documented functionality, not a security issue |

---

## Key Results

| Platform | Capture Method | Evasion Rate | Artefacts |
|----------|---------------|-------------|-----------|
| Windows 11 23H2 | Chrome 122 / Edge 122 | 100% | None |
| Windows 10 22H2 | Chrome 122 / Firefox 123 | 100% | None |
| macOS 14.3 (Sonoma) | Chrome 122 / Safari 17.3 | 100% | None |
| macOS 26.3.1 | `screencapture` / `CGWindowListCreateImage` | 100% | None |
| Linux (X11/Wayland) | — | N/A | Not vulnerable |

macOS 26.3.1 remains fully vulnerable despite Apple's documented ScreenCaptureKit changes in macOS 15.

---

## Vendor Classifications

Both OS vendors reviewed the reported behaviour and classified it as by-design rather than a security vulnerability:

- **Apple Product Security** (28 Mar 2026): *"The behaviour is consistent with Apple's documented functionality for NSWindow.SharingType.none … does not bypass a security boundary."*
- **Microsoft MSRC** (2 Apr 2026): *"Concluded as not a security vulnerability … categorized as by-design behavior."*

This paper does not dispute either classification. The vulnerability is framed as a **downstream display-fidelity failure** in capture-dependent systems that treat OS screen-capture output as equivalent to the physical display. See §VII-B of the paper for full security boundary analysis.

---

## Responsible Disclosure

| Party | Notified | Response | Classification |
|-------|----------|----------|----------------|
| Apple Product Security | Mar 2026 | Mar 2026 | Consistent with documented functionality; not a security issue |
| Microsoft MSRC | Feb 2026 | Apr 2026 | By-design behaviour; not a security vulnerability |

---

## Research Methodology

Developed using Claude Code powered by Claude Opus 4.6 (1M context window). The AI-assisted methodology, capability uplift measurement, and dual-use implications are documented in §VIII-G of the paper.

---

## Citation

```bibtex
@misc{abedini2026invisible,
  author       = {Abedini, Mohammad Raouf},
  title        = {{The Invisible Window: Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems}},
  year         = {2026},
  doi          = {10.5281/zenodo.20376495},
  url          = {https://doi.org/10.5281/zenodo.20376495}
}
```

---

## License

CC BY 4.0. Proof-of-concept source code is not publicly released due to dual-use considerations; available to verified security researchers and proctoring vendors on request. See §VII-E of the paper.
