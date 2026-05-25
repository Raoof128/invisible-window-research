# AGENT.md — The Invisible Window Research Repository

## Project Identity

**Paper**: "The Invisible Window: Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems"  
**Author**: Mohammad Raouf Abedini  
**Affiliation**: Department of Computing, Macquarie University, Sydney, Australia  
**Email**: mohammadraouf.abedini@students.mq.edu.au  
**ORCID**: 0009-0000-6214-258X  
**Preprint DOI**: 10.5281/zenodo.20319832  
**Format**: IEEE Conference (IEEEtran `[conference]`)  
**Current page count**: 13 pages (compiled)

---

## Repository Layout

```
paper/
  main.tex          # Primary LaTeX source — all edits go here
  references.bib    # BibTeX — 51 citations, all author names verified
  main.pdf          # Compiled output
  figures/          # Forensic diff images (do not modify)
  Makefile          # `make` or `latexmk -pdf main.tex`
docs/
  invisible-window-paper.md        # Markdown draft (reference only, not canonical)
  invisible-window-references.md   # Reference list (reference only)
  citation_verification_report.json  # Citation audit — all errors already fixed in bib
poc/
  windows/          # C proof-of-concept (do not distribute)
  macos/            # Swift proof-of-concept (do not distribute)
  linux/            # X11 analysis
reasoning-engine/   # Deep research MCP server
CHANGELOG.md        # Change log — update after every edit session
AGENT.md            # This file
```

---

## Canonical Source of Truth

`paper/main.tex` is the **only canonical version** of the paper. The markdown file at `docs/invisible-window-paper.md` is a legacy draft and may be out of date — do not use it as a reference for current paper content.

---

## Build Instructions

```bash
cd paper/
latexmk -pdf main.tex          # Build PDF
latexmk -pdf -g main.tex       # Force full rebuild
make                           # Equivalent via Makefile
```

**Required packages**: `IEEEtran`, `orcidlink`, `tikz`, `listings`, `booktabs`, `tabularx`, `hyperref`, `amsmath`, `url`. All are standard in TeX Live 2024+.

**Expected output**: 13 pages, 0 errors, 0 overfull boxes.

---

## Section Structure (current)

| Section | Title |
|---------|-------|
| §I | Introduction |
| §II | Background |
| §III | Threat Model |
| §IV | Attack Design |
| §V | Evaluation |
| §VI | Countermeasures |
| §VII | Ethical Considerations |
| §VIII | Related Work |
| §IX | AI Capability Uplift: A Case Study |
| §X | Conclusion |

**Note**: §IX is a standalone section (not part of Related Work). Cross-references to this section should use `Section~IX`, not `Section~VIII-G`.

---

## Citation Rules (MANDATORY)

- All 51 BibTeX entries in `references.bib` have been verified via Scholar Gateway and web crawl.
- Author name errors (11 entries, 24 field errors) were corrected as of 2026-03-26. Do not revert.
- **Never generate new BibTeX from memory.** Use Scholar Gateway MCP or CrossRef API to verify any new citations before adding them.
- Mark unverified citations as `% VERIFY:` in the bib file and note them in CHANGELOG.md.

---

## Writing Constraints

1. **Abstract**: No citations. Target 200–220 words. Must include the qualifier "in our controlled evaluation" when claiming no detectable behavioural anomalies.
2. **Statistical claims**: Always include degrees of freedom for t-tests (`t(df) = value`). Do not claim statistical significance on n=8 data without explicit caveats.
3. **Evasion rate**: Always accompany "100% evasion" with a mechanistic explanation (binary flag, not probabilistic).
4. **macOS 26**: Clarify this was tested via `CGWindowListCreateImage` and `screencapture`, not browser `getDisplayMedia()`, when that distinction matters.
5. **Variant D** (click-through): Is cross-referenced in §VI-C. Do not remove it without updating that cross-reference.
6. **Section numbering**: §IX = AI Capability Uplift, §X = Conclusion. Any new section must slot correctly and update the intro roadmap (§I, final paragraph).

---

## Dual-Use and Ethics Constraints

- Do not publicly release PoC source code. Restricted to verified security researchers via email.
- The paper follows coordinated disclosure: both Apple Product Security and Microsoft MSRC were notified and responded (vendor responses documented in §VII-B and Table 3).
- Apple and Microsoft both classified the behaviour as by-design, not a security vulnerability. The paper explicitly does not dispute this and frames the issue as a downstream trust mismatch in capture-dependent systems.
- Any changes to §VII (Ethical Considerations) should preserve this framing.

---

## Venue Considerations

| Venue | Page Limit | Fit |
|-------|-----------|-----|
| USENIX Security | 13 pages body | ✅ fits as-is |
| IEEE S&P (Oakland) | 10 pages | ✗ needs ~3 pages cut |
| ACM CCS | 12 pages | ✗ needs ~1 page cut |
| NDSS | 12 pages | ✗ needs ~1 page cut |

Current submission target is **not yet decided**. Do not hardcode a venue name in the paper.

---

## Postflight Requirement

After any editing session on this repo, append an entry to `CHANGELOG.md` using the `Raouf:` template:

```
## Raouf: YYYY-MM-DD (AEST) — [scope]

### Scope
[what was changed]

### Summary
[2–3 sentence summary]

### Files Changed
- `file` — description

### Verification
- [build result or test result]

### Follow-ups
- [any open items]
```
