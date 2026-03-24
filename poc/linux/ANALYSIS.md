# Linux Display Affinity Analysis

**Author:** Mohammad Raouf Abedini
**Date:** 24 March 2026

## Executive Summary

Linux (X11 and Wayland) does NOT have OS-level display affinity APIs equivalent to Windows `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)` or macOS `NSWindow.SharingType.none`. The Invisible Window attack is **not fully feasible on Linux**.

## X11

### Approach A: Override-Redirect Windows
- `override_redirect = True` bypasses the window manager
- Window is not managed (no title bar, no minimize/maximize)
- **Capture result:** Most X11 screenshot tools (scrot, gnome-screenshot, xdg-desktop-portal) still capture override-redirect windows because they read from the root window composite
- **Verdict:** Does NOT achieve capture invisibility

### Approach B: XComposite Redirection
- `XCompositeRedirectWindow(dpy, win, CompositeRedirectManual)` tells the compositor that the application will handle compositing for this window
- With manual redirect, the window's pixels may not be included in the root window pixmap by some compositors
- **Capture result:** Depends on compositor. On Mutter (GNOME), Picom, and KWin, the redirected window is still captured. Some minimal compositors may miss it.
- **Verdict:** UNRELIABLE — compositor-dependent behavior

### Approach C: XShape / Input-Only Windows
- XShape extension can create irregularly shaped windows
- Input-only windows have no visual representation
- Neither achieves the goal: we need visible content that is NOT captured

## Wayland

### No Display Affinity Mechanism
Wayland's security model is fundamentally different from X11:
- Applications cannot access other surfaces' content (unlike X11's permissive model)
- Screen capture goes through `xdg-desktop-portal`, which requests compositor-level access
- The compositor has full authority over what is captured
- There is NO client-side API to exclude a surface from capture

### PipeWire Screen Capture
Modern Wayland desktops use PipeWire for screen capture:
- `xdg-desktop-portal` negotiates capture with the user
- The compositor (Mutter, KWin, Sway) provides the capture stream
- Applications cannot influence what the compositor includes in the stream
- **Verdict:** NOT FEASIBLE on Wayland

## Comparison Table

| Platform | API | Full Evasion | Partial Evasion |
|----------|-----|-------------|-----------------|
| Windows 10/11 | `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)` | **YES** | — |
| macOS 12-14 | `NSWindow.SharingType.none` | **YES** | — |
| macOS 15-26 | `NSWindow.SharingType.none` | **YES** (tested) | — |
| Linux/X11 | Override-redirect + XComposite | NO | Marginal |
| Linux/Wayland | None | NO | NO |

## Implications for the Paper

1. The Invisible Window attack is a **Windows and macOS** vulnerability
2. Linux's lack of display affinity APIs is actually a security advantage in this context
3. The paper should note that Linux-based proctoring environments are not vulnerable to this specific attack class
4. This does NOT mean Linux is immune to proctoring evasion — VM-based attacks, second monitors, and other techniques still apply

## Why Linux Differs

The fundamental difference is architectural:
- **Windows:** DWM (Desktop Window Manager) allows per-window capture exclusion flags because Microsoft designed the compositing pipeline to support DRM content protection
- **macOS:** WindowServer allows `sharingType` control because Apple designed it for content protection (originally for iTunes/DRM video playback)
- **Linux/X11:** The X server has no per-window capture exclusion — any client can read any window's pixels via XGetImage
- **Linux/Wayland:** The compositor owns ALL rendering. Clients have no mechanism to influence capture behavior. This is by security design.

The irony: Linux's less sophisticated compositing model (X11) and more secure compositing model (Wayland) both happen to be immune to the display affinity attack — X11 because it lacks the feature, Wayland because it prevents client-side influence over capture.
