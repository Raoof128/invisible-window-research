# The Invisible Window

**Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems**

> How AI-powered tools exploit OS-level display APIs to enable undetectable AI-assisted academic misconduct — and what defenses are needed.

**Author:** Mohammad Raouf Abedini
**Affiliation:** Department of Computing, Macquarie University, Sydney, Australia
**Contact:** mohammadraouf.abedini@students.mq.edu.au | [raoufabedini.dev](https://raoufabedini.dev)

---

## Overview

This repository contains the research artifacts for an IEEE-format security paper demonstrating a fundamental architectural vulnerability in browser-based proctoring systems. The WebRTC `getDisplayMedia()` API implicitly trusts the OS compositing pipeline — but documented OS APIs (`SetWindowDisplayAffinity` on Windows, `NSWindow.SharingType.none` on macOS) allow windows to be visible on the physical display while invisible to ALL screen capture.

**Key finding:** Commercial AI-powered tools (Cluely, Interview Coder) already exploit this to embed Claude/GPT-4 as invisible real-time cheating assistants. Our PoC achieves 100% evasion on all tested platforms including macOS 26 — contradicting the prevailing assumption that Apple mitigated this vulnerability.

## Repository Structure

```
├── docs/
│   ├── invisible-window-paper.md      # Full IEEE-format manuscript (8,672 words, 53 refs)
│   ├── invisible-window-references.md # 42 verified academic references (Harvard Australian)
│   └── anthropic-grant-pitch.md       # Multi-program grant strategy
├── poc/
│   ├── macos/                         # macOS PoC (Swift, sharingType = .none)
│   │   ├── invisible_window.swift     # Source code
│   │   ├── build.sh                   # Build script
│   │   ├── capture_verify.py          # Capture verification tool
│   │   ├── comprehensive_test.py      # 22-subtest validation suite
│   │   └── RESULTS.md                 # Evaluation results (Rev 3 — FULL EVASION confirmed)
│   ├── windows/                       # Windows PoC (C/Win32, WDA_EXCLUDEFROMCAPTURE)
│   │   ├── invisible_window.c         # Source code
│   │   └── build.bat                  # MSVC + GCC build script
│   └── linux/                         # Linux analysis (NOT vulnerable)
│       ├── invisible_window_x11.c     # X11 experimental PoC
│       ├── build.sh                   # Build script
│       └── ANALYSIS.md               # Analysis: Linux lacks display affinity APIs
├── reasoning-engine/                  # ACPR reasoning backend (MCP server)
├── CHANGELOG.md                       # Full project history
└── README.md                          # This file
```

## Key Results

| Platform | API | Evasion Rate | Artifacts |
|----------|-----|-------------|-----------|
| Windows 10/11 | `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)` | 100% | None |
| macOS 14 | `NSWindow.SharingType.none` | 100% | None |
| macOS 26 | `NSWindow.SharingType.none` | **100%** | **None** |
| Linux/X11 | N/A (no equivalent API) | N/A | NOT VULNERABLE |
| Linux/Wayland | N/A | N/A | NOT VULNERABLE |

**Novel finding:** macOS 26.3.1 remains fully vulnerable despite Apple's documented ScreenCaptureKit changes in macOS 15. Pixel-level A/B forensic comparison (1,170,560 pixels) confirms 100% evasion with zero artifacts.

## Research Methodology

This research was conducted using **Claude Code powered by Claude Opus 4.6 (1M context)** as the primary research instrument — from literature review to PoC development to forensic evaluation.

## Ethical Statement

This research follows coordinated vulnerability disclosure principles. The attack vector is already commercially exploited by multiple products. This work provides the first formal academic analysis to enable informed defenses. See Section VII of the paper for the full ethical framework.

## License

This repository is private and contains unpublished research. All rights reserved until publication.
