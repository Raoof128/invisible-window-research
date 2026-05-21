# Submission Guide — The Invisible Window
## arXiv and TechRxiv

---

## TechRxiv (IEEE Preprint Server) — Primary Target

TechRxiv accepts PDF-only submissions. No source package required.

### Steps
1. Go to [techrxiv.org](https://www.techrxiv.org) → Submit Article
2. Upload `paper/main.pdf`
3. Fill in metadata (see below)
4. Select CC BY 4.0 license
5. Submit

### Metadata

| Field | Value |
|---|---|
| Title | The Invisible Window: Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems |
| Author | Mohammad Raouf Abedini |
| ORCID | 0009-0000-6214-258X |
| Affiliation | Department of Computing, Macquarie University, Sydney, Australia |
| Subject Area | Computer Science → Computer Security and Cryptography |
| Keywords | AI-assisted cheating, display affinity, screen capture evasion, WebRTC proctoring, generative AI misuse, academic integrity, AI safety, responsible disclosure |
| License | CC BY 4.0 |

### Abstract (paste as-is)
Copy from `paper/main.tex` between `\begin{abstract}` and `\end{abstract}`, stripping LaTeX markup.

---

## arXiv — Secondary Target

arXiv requires LaTeX source. Use the submission archive below.

### Submission Package

Build with:
```bash
cd paper && make arxiv-archive
```

Or manually:
```bash
cd paper
tar -czf ../arxiv-submission.tar.gz \
  main.tex main.bbl references.bib \
  figures/windows-diff.png figures/macos-capture.png
```

Contents: `main.tex`, `main.bbl`, `references.bib`, `figures/`  
**Exclude:** `main.bib`, `main.pdf`, `main.aux`, `*.log`, `*.fls`, `*.fdb_latexmk`, `*.blg`, `*.out`, `*.synctex.gz`

### Category
- **Primary:** cs.CR (Cryptography and Security)
- **Cross-list:** cs.AI (Artificial Intelligence), cs.CY (Computers and Society)

### Comments Field
```
13 pages, 3 figures, 3 tables
```

### License
CC BY 4.0 (compatible with IEEE later if needed)

---

## Pre-Submission Checklist

- [x] 53 citations — all resolved, none unused
- [x] 0 overfull hbox warnings
- [x] 3 figures, 3 tables, all cross-references resolve
- [x] Abstract: ~1,889 chars (under arXiv 1,920 limit)
- [x] All fonts Type 1, embedded, subsetted
- [x] `orcidlink` package compatible with TeX Live 2023+
- [x] No build artifacts in submission archive
- [x] pdflatex compilation tested — 13 pages, clean
- [x] ORCID linked: 0009-0000-6214-258X
- [x] Responsible disclosure timeline accurate (Apple + MSRC only)
- [x] Vendor response section (§VII-B) with formal classifications
- [x] Proctoring vendor claims removed (only contacted OS vendors)
- [ ] cs.CR endorsement (needed on arXiv if first-time submitter in cs.CR)

## Notes

- IEEEtran document class is fully supported on arXiv
- arXiv submissions before 14:00 ET are announced same day at ~20:00 ET
- TechRxiv DOI is issued immediately on submission
- Zenodo DOI: `10.5281/zenodo.20195134` (update record with latest PDF before submitting elsewhere)
