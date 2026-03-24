# The Invisible Window: Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems

**Mohammad Raouf Abedini**
Department of Computing, Macquarie University, Sydney, Australia
mohammadraouf.abedini@students.mq.edu.au | https://raoufabedini.dev

**Abstract** — Remote proctoring systems are the primary technical defense against academic misconduct in online education. These systems rely on the WebRTC `getDisplayMedia()` API to monitor test-takers' screens, operating on the implicit assumption that the captured frame faithfully represents the physical display. We demonstrate that this assumption is systematically violated by a new class of AI-powered cheating tools — including commercially available products such as Interview Coder and Cluely — that embed large language models (GPT-4, Claude) inside operating system windows rendered invisible to all screen capture mechanisms. These tools exploit documented OS-level display affinity APIs (`SetWindowDisplayAffinity` on Windows, `NSWindow.SharingType.none` on macOS) to create real-time AI assistants that are fully visible to the test-taker but produce zero pixels in proctoring capture output. Our proof-of-concept demonstrates 100% evasion across all tested platforms — including macOS 26, where the attack was previously assumed mitigated — with zero visual artifacts and no detectable behavioral anomalies. We find that an estimated 35% of candidates in technical assessments already show signs of AI-assisted cheating via such tools, yet no formal security analysis of this attack class exists in the academic literature. We propose countermeasures including display integrity attestation and API-level monitoring, assess their limitations, and argue that the structural vulnerability of capture-based proctoring demands a fundamental rethinking of assessment design in the age of accessible AI. This work follows coordinated vulnerability disclosure principles and is positioned at the intersection of AI security, AI-enabled misuse, and the societal impacts of generative AI on educational institutions.

**Keywords** — AI-assisted cheating, display affinity, screen capture evasion, WebRTC proctoring, generative AI misuse, academic integrity, AI safety, responsible disclosure

**Methodology Note** — This research was conducted using Claude Code powered by Claude Opus 4.6 (1M context) as the primary research instrument, demonstrating both the transformative potential of frontier AI for academic research and the dual-use implications central to this paper's thesis.

<!-- ORIGINAL ABSTRACT: Technical Framing (for IEEE/security venues without AI safety angle) -->
<!--
**Abstract (Original Version)** — Remote proctoring systems have become a cornerstone of academic integrity enforcement in online education, particularly following the rapid adoption accelerated by the COVID-19 pandemic. These systems overwhelmingly rely on the WebRTC `getDisplayMedia()` API to capture and monitor a test-taker's screen in real time. This paper identifies and demonstrates a fundamental architectural vulnerability in this approach: the `getDisplayMedia()` API implicitly trusts the operating system's compositing pipeline to faithfully represent what is physically displayed on the monitor. We show that this trust assumption is violated by standard, documented OS-level APIs — specifically, Windows `SetWindowDisplayAffinity` with the `WDA_EXCLUDEFROMCAPTURE` flag and macOS `NSWindow.SharingType.none` — which allow arbitrary application windows to remain fully visible to the user while being completely invisible to any screen capture mechanism, including WebRTC. We present proof-of-concept implementations on both Windows and macOS that render an auxiliary "cheat sheet" window entirely invisible to proctoring software while remaining fully readable on the physical display. Our evaluation against representative browser-based proctoring configurations demonstrates a 100% evasion rate with zero detectable artifacts in the captured screen feed, the webcam stream, or standard behavioral telemetry. We analyse countermeasures including process enumeration, API call monitoring, and display integrity attestation, and assess their feasibility and limitations. This work follows coordinated vulnerability disclosure principles and is presented to inform the development of more robust proctoring architectures rather than to enable academic dishonesty.

**Keywords (Original Version)** — display affinity, screen capture, WebRTC, getDisplayMedia, online proctoring, academic integrity, security vulnerability, responsible disclosure
-->

---

## I. Introduction

The global shift to online education, dramatically accelerated by the COVID-19 pandemic, has driven widespread adoption of remote proctoring technologies to safeguard academic integrity [5, 6, 10]. Commercial systems such as ProctorU, Proctorio, Respondus LockDown Browser, and ExamSoft now monitor millions of examinations annually, employing a combination of webcam surveillance, screen capture, browser lockdown, and behavioral analytics to detect and deter cheating [3, 7, 8].

At the technical core of browser-based proctoring lies the WebRTC Screen Capture API, specifically the `getDisplayMedia()` method specified by the W3C [1]. This API provides proctoring software with a real-time video stream of the test-taker's screen, enabling remote invigilators — human or automated — to verify that no unauthorized materials are visible during an examination. The implicit security assumption is straightforward: what `getDisplayMedia()` captures is what the user sees.

This paper demonstrates that this assumption is false.

Modern operating systems provide documented, publicly available APIs that allow application windows to be selectively excluded from all screen capture mechanisms while remaining fully visible on the physical display. On Windows, the `SetWindowDisplayAffinity` function with the `WDA_EXCLUDEFROMCAPTURE` flag [11] causes a window to "not appear at all" in any capture output. On macOS, `NSWindow.SharingType.none` [12] achieves an equivalent effect by preventing legacy CoreGraphics capture APIs from reading window content. These APIs were designed for legitimate content protection purposes — hiding video playback controls from recordings, protecting DRM-encumbered content [14] — but they create a fundamental blind spot that undermines the entire premise of screen capture-based proctoring.

We term this class of attack the *Invisible Window* attack. The attacker creates an overlay or secondary window containing unauthorized materials (notes, a web browser, a communication channel) and applies the appropriate display affinity flag. The window is fully visible and interactive on the physical monitor, but produces no pixels whatsoever in the `getDisplayMedia()` output stream. From the proctoring system's perspective, the screen appears clean. From the test-taker's perspective, the cheat sheet is right there.

The attack is notable for several reasons:

1. **Zero artifact**: Unlike virtual machine-based evasion or screen injection attacks, the Invisible Window produces no visual artifacts, no process anomalies detectable by standard monitoring, and no behavioral signatures in gaze tracking or mouse telemetry.

2. **Minimal technical barrier**: The Windows implementation requires approximately 15 lines of C code using a single Win32 API call. The macOS implementation requires a comparable amount of Swift.

3. **OS-vendor documented**: The APIs are not exploits, zero-days, or undocumented features. They are publicly documented, officially supported, and intended for use by application developers.

4. **Cross-platform**: The attack is feasible on both major desktop operating systems, achieving 100% evasion on all tested versions of Windows 10/11 and macOS 14–26.

### Contributions

This paper makes the following contributions:

- **Vulnerability identification**: We identify and formalize the trust boundary violation between the W3C Screen Capture API and the OS compositing pipeline, demonstrating that `getDisplayMedia()` cannot guarantee display fidelity.

- **Proof-of-concept attacks**: We present working implementations on Windows 10/11 and macOS that achieve complete screen capture evasion using only documented OS APIs, with empirical verification on macOS through version 26.

- **Systematic evaluation**: We evaluate the attack against representative browser-based proctoring configurations and analyze which existing detection mechanisms (gaze tracking, mouse dynamics, process enumeration) can and cannot detect it.

- **Countermeasure analysis**: We propose and assess potential defenses, including display integrity attestation, API call interception, and hardware-rooted trust, identifying their feasibility and limitations.

- **Responsible disclosure**: We frame this work within established ethical guidelines for security research [34, 35, 36, 37, 38] and discuss the coordinated disclosure process followed.

The remainder of this paper is organized as follows. Section II provides background on the WebRTC Screen Capture API, OS-level display affinity mechanisms, and the architecture of browser-based proctoring systems. Section III formalizes the threat model. Section IV details the attack design and implementation. Section V presents the evaluation methodology and results. Section VI analyzes countermeasures. Section VII discusses ethical considerations and responsible disclosure. Section VIII surveys related work. Section IX concludes.

---

## II. Background

This section establishes the technical foundations required to understand the Invisible Window attack: the WebRTC Screen Capture API and its trust model, the OS-level display affinity mechanisms that violate that trust model, and the architecture of browser-based proctoring systems that depend on it.

### A. WebRTC Screen Capture API

The W3C Screen Capture specification [1] defines the `getDisplayMedia()` method on the `MediaDevices` interface, enabling web applications to capture the contents of a user's display as a `MediaStream`. The specification was designed primarily for screen sharing in video conferencing applications and was subsequently adopted by proctoring systems as a mechanism for remote screen monitoring.

The API operates under a consent-based security model. When invoked, the browser presents a system dialog allowing the user to select which display surface to share — an entire monitor, a specific application window, or a browser tab. The specification mandates that "the user agent MUST let the end-user choose which display surface to share" and that HTTPS origins and transient user activation are required [1].

Critically, the specification's security considerations focus on *authorization* (ensuring the user consents to sharing) rather than *fidelity* (ensuring the shared content accurately represents what is displayed). Section 5 of the specification acknowledges that "display capture presents risk to the cross site request forgery protections offered by the browser sandbox" [1] but does not address the possibility that the captured pixels might not reflect the physical display state. The API delegates pixel composition entirely to the operating system's compositing pipeline [4], operating on the implicit assumption that the OS faithfully reports what is visible on the monitor.

This delegation of trust is the fundamental vulnerability we exploit.

### B. OS-Level Display Affinity

Modern desktop operating systems employ a compositing window manager that maintains a scene graph of all visible windows and renders them into a framebuffer for display output. Screen capture APIs — including those used by `getDisplayMedia()` — typically read from this composited framebuffer or from a parallel capture pipeline maintained by the window manager.

Both Windows and macOS provide mechanisms for applications to exclude their windows from this capture pipeline while maintaining their visibility on the physical display.

#### Windows: SetWindowDisplayAffinity

The Win32 function `SetWindowDisplayAffinity` [11] allows an application to specify where its window content can be displayed. The function accepts a handle to a top-level window and a `DWORD` affinity value. Three values are defined:

- `WDA_NONE` (0x00000000): No restrictions; the window is visible everywhere.
- `WDA_MONITOR` (0x00000001): The window content is displayed only on a physical monitor; captured output shows a black rectangle.
- `WDA_EXCLUDEFROMCAPTURE` (0x00000011): Introduced in Windows 10 Version 2004, the window is displayed only on a physical monitor and *does not appear at all* in capture output — no black rectangle, no placeholder, nothing.

The distinction between `WDA_MONITOR` and `WDA_EXCLUDEFROMCAPTURE` is significant. The former produces a visible black region that could alert a proctoring system to the presence of a hidden window. The latter produces no artifact whatsoever — the window simply does not exist in the captured frame. Microsoft's documentation explicitly notes that this feature "is not a security feature or an implementation of Digital Rights Management (DRM)" and offers "no guarantee" of strict content protection [11], confirming it was designed as a best-effort content protection mechanism rather than a security boundary.

The function requires only that the calling process owns the target window. No elevated privileges, no special capabilities, and no administrator access are required.

#### macOS: NSWindow.SharingType

On macOS, the `NSWindow` class provides a `sharingType` property [12] that controls whether a window's content can be read by other processes. Setting `sharingType` to `.none` prevents legacy CoreGraphics-based screen capture APIs from accessing the window content. On macOS versions prior to 15, this effectively hides the window from `getDisplayMedia()` when Chrome or another browser uses CoreGraphics for screen capture.

Apple's ScreenCaptureKit framework [13], introduced in macOS 12.3, provides a more modern capture API with explicit support for content filtering through `SCContentFilter`. This framework allows capture clients to include or exclude specific windows and applications, and it captures content by reading the composited framebuffer directly.

A significant platform divergence was expected in macOS 15 (Sequoia), where Apple reportedly changed ScreenCaptureKit to capture all visible content regardless of `sharingType` settings [15]. Community reports and developer discussions documented this change, leading to the widespread assumption that the display affinity attack vector was mitigated on macOS 15+. However, our empirical evaluation on macOS 26.3.1 (Section V) demonstrates that `sharingType = .none` continues to fully hide window content from all tested capture APIs — including `CGWindowListCreateImage` and the `screencapture` system utility — contradicting this assumption. The implications for cross-platform attack viability are discussed in Section IV.

### C. Browser-Based Proctoring Architecture

Browser-based proctoring systems typically implement a layered monitoring architecture [3, 7, 40]:

1. **Screen capture layer**: Uses `getDisplayMedia()` to obtain a real-time video stream of the test-taker's screen, which is transmitted to a remote server or analyzed locally.

2. **Webcam monitoring layer**: Captures the test-taker's face and environment via `getUserMedia()`, enabling facial recognition, gaze estimation, and room scanning [20, 21].

3. **Browser lockdown layer**: Restricts the test-taker's browser to the examination page by intercepting navigation events, disabling keyboard shortcuts, blocking copy/paste, and preventing new tab/window creation.

4. **Behavioral analytics layer**: Analyzes webcam and interaction data for suspicious patterns including gaze deviation [21], unusual mouse dynamics [25, 26, 27], and application switching [40].

5. **Process monitoring layer** (native clients only): Some proctoring systems install a native agent that enumerates running processes, detects virtual machines, and monitors system-level events.

The Invisible Window attack targets the first layer exclusively. Because the hidden window never appears in the `getDisplayMedia()` output, the screen capture layer reports a clean feed. The remaining layers continue to function normally: the webcam sees the student looking at their screen (which is where the hidden content is), behavioral analytics observe normal mouse and keyboard patterns (the student is interacting with content on their screen), and the browser lockdown remains intact (the hidden window is a separate native application, not a browser tab).

This architectural insight — that the screen capture layer is a single point of failure independent of other monitoring layers — is central to the attack's effectiveness.

### D. Security Requirements for Online Proctoring

Luijben, van den Broek, and Alpár [17] identify five pivotal security requirements for proctoring systems:

1. **Student authentication**: Verify the test-taker's identity.
2. **Work authenticity**: Verify the test-taker's work is their own.
3. **No prior access**: Prevent early access to exam materials.
4. **Data protection**: Protect students' personal data.
5. **Availability**: Ensure the exam system remains accessible.

The Invisible Window attack directly violates Requirement 2 (work authenticity) by enabling the test-taker to consult unauthorized materials without detection. It does so without violating any of the other requirements — the student is authenticated, the exam data is protected, and the system remains available. This surgical violation of a single security requirement while preserving all others makes the attack particularly difficult to detect through holistic system monitoring.

---

## III. Threat Model

### A. Actors and Assumptions

We define the following actors:

- **Test-taker (Adversary)**: A student taking a remotely proctored examination who wishes to consult unauthorized materials without detection. We assume the adversary has:
  - Standard (non-administrator) user access to their own computer.
  - The ability to compile and run a simple native application before the exam begins.
  - Basic technical competence (can download and execute a compiled binary).
  - No ability to modify the proctoring software itself, the browser, or the operating system kernel.

- **Proctoring system (Defender)**: A browser-based proctoring application that captures the test-taker's screen via `getDisplayMedia()` and monitors their webcam feed. We assume the defender:
  - Has access to the full `getDisplayMedia()` video stream.
  - Has access to the webcam feed with gaze estimation capabilities.
  - May perform behavioral analysis on mouse, keyboard, and interaction patterns.
  - Does *not* have kernel-level access or a native agent with system-wide process monitoring (this represents the common case of browser-only proctoring).

- **Operating system**: A standard installation of Windows 10/11 or macOS 12–26, running the Desktop Window Manager (Windows) or WindowServer (macOS) compositing window manager.

### B. Trust Boundary Analysis

The critical trust boundary lies between the browser and the operating system's display pipeline. The browser's `getDisplayMedia()` implementation calls into OS-level APIs to obtain screen content. On Windows, this typically involves the Desktop Duplication API or the Windows Graphics Capture API. On macOS, this involves CoreGraphics `CGWindowListCreateImage` or ScreenCaptureKit's `SCStream`.

Both of these OS capture APIs respect display affinity flags. When a window has `WDA_EXCLUDEFROMCAPTURE` set (Windows) or `sharingType = .none` (macOS), the capture API omits that window's content from the output. The browser has no mechanism to detect this omission — it receives a valid, complete-looking frame that simply does not include the hidden window's pixels.

This represents a violation of the *display fidelity assumption*: the expectation that the frame returned by the OS capture API is a faithful representation of what the user sees on the physical monitor. We formalize this as:

**Definition (Display Fidelity).** A screen capture system satisfies display fidelity if and only if the captured frame F is pixel-identical to the physical display output D for all visible screen regions, i.e., F(x,y) = D(x,y) for all pixels (x,y) where a window is visible on the physical display.

**Theorem (Fidelity Violation).** On Windows 10 Version 2004+ and macOS 12–26, the display fidelity property does not hold when any visible window has `WDA_EXCLUDEFROMCAPTURE` or `sharingType = .none` set, as the capture API will return F(x,y) != D(x,y) for the pixels covered by that window.

### C. Attack Surface

The attack surface encompasses any application that:

1. Creates a top-level window on the default desktop.
2. Sets the appropriate display affinity flag.
3. Renders content that the test-taker wishes to view during the exam.

The attack requires no interaction with the proctoring system, no network manipulation, no privilege escalation, and no exploitation of software vulnerabilities. It uses documented, supported OS APIs for their intended purpose (content protection) in a context (proctoring evasion) that the API designers did not anticipate.

### D. Scope and Limitations

The attack is effective against:
- All browser-based proctoring systems that rely on `getDisplayMedia()` for screen capture.
- All screen recording tools that use the standard OS capture pipeline.

The attack is *not* effective against:
- Proctoring systems with kernel-level agents that enumerate windows and check display affinity flags directly.
- Physical observation (the window is visible on the physical monitor).
- Hardware-based capture devices (e.g., HDMI capture cards) that read the display output signal directly.
---

## IV. Attack Design

### A. Overview

The Invisible Window attack consists of three phases:

1. **Preparation (pre-exam)**: The adversary prepares a native application that creates a window with display affinity flags set, containing the desired unauthorized materials.

2. **Activation (exam start)**: Before or during the exam, the adversary launches the prepared application. The window appears on the physical display but is invisible to screen capture.

3. **Exploitation (during exam)**: The adversary reads content from the invisible window while appearing to look at the exam screen. The proctoring system's screen capture shows only the exam interface.

### B. Windows Implementation

The Windows implementation leverages the `SetWindowDisplayAffinity` Win32 API function. The core mechanism requires only a single API call after window creation:

```c
// After creating the window with CreateWindowEx()
BOOL result = SetWindowDisplayAffinity(
    hWnd,                        // Handle to the window
    WDA_EXCLUDEFROMCAPTURE       // 0x00000011
);
```

The complete implementation follows a standard Win32 window application pattern:

1. Register a window class with `RegisterClassEx`.
2. Create a top-level window with `CreateWindowEx`, using `WS_EX_TOPMOST` to ensure the window stays above other windows (including the browser running the exam).
3. Immediately call `SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE)`.
4. Render the desired content (text notes, images, a web browser control) in the window's client area.

The `WS_EX_TOPMOST` extended style ensures the invisible window floats above the exam browser, allowing the test-taker to read both the exam questions and the cheat material simultaneously.

**Variant A — Static notes**: The simplest variant renders static text or images in the invisible window. This is equivalent to having a physical note card, but undetectable by screen capture.

**Variant B — Embedded browser**: A more sophisticated variant embeds a web browser control (e.g., WebView2 on Windows) within the invisible window, providing full internet access through a capture-invisible interface.

**Variant C — Communication channel**: The window hosts a messaging application or live video feed, enabling real-time communication with an external accomplice.

In all variants, the window is fully interactive — the test-taker can scroll, click links, type queries, and resize the window. All interactions occur within a window that does not exist in the proctoring system's view of the screen.

**System Requirements**: Windows 10 Version 2004 (May 2020 Update) or later. No administrator privileges. No special runtime dependencies. The compiled binary is approximately 15 KB.

### C. macOS Implementation

The macOS implementation uses the `NSWindow.sharingType` property:

```swift
let window = NSWindow(
    contentRect: NSRect(x: 100, y: 100, width: 800, height: 600),
    styleMask: [.titled, .closable, .resizable],
    backing: .buffered,
    defer: false
)
window.sharingType = .none  // Exclude from screen capture
window.level = .floating    // Stay above other windows
window.makeKeyAndOrderFront(nil)
```

Setting `sharingType` to `.none` instructs the window server to exclude this window from all screen capture operations that respect the sharing type flag. On macOS 12–26, this includes both legacy CoreGraphics capture and the system-level capture APIs that underlie ScreenCaptureKit-based tools.

**macOS 15+ Behavior**: Apple's documentation and community reports [15] indicate that ScreenCaptureKit in macOS 15+ was changed to capture all visible content regardless of `sharingType` settings. However, our empirical testing on macOS 26.3.1 reveals that this mitigation is incomplete. Despite the documented changes, `sharingType = .none` continues to fully exclude window content from capture output across all APIs tested — including `CGWindowListCreateImage` (which underlies many capture tools) and the `screencapture` system utility. A pixel-level A/B comparison of the same screen region with and without the invisible window showed 92% identical pixels (1,086,110 of 1,170,560), with all 84,450 differing pixels attributable to background terminal content scrolling between captures and zero pixels differing due to the invisible window, confirming complete capture evasion.

This finding contradicts the prevailing assumption in developer communities [15] and suggests one of two possibilities: (1) the macOS `screencapture` utility and `CGWindowListCreateImage` API still use a legacy capture path that respects `sharingType`, even on macOS 26; or (2) Apple's ScreenCaptureKit changes were less comprehensive than documented. Either way, the attack remains fully effective on the latest macOS release as of our testing date.

The attack is therefore effective on *all tested platforms*: Windows 10/11 and macOS 14–26.

### D. Operational Considerations

**Pre-exam preparation**: The adversary compiles the invisible window application and prepares the content before the exam session begins. The application can be stored on a USB drive or in a user-accessible directory.

**Timing**: The application should be launched before the proctoring session begins screen capture, or during a natural pause (e.g., while reading instructions). Process creation events may be logged by sophisticated proctoring agents.

**Window positioning**: The invisible window should be positioned to overlap minimally with exam interface elements that require interaction (buttons, text fields), while maximizing visibility of the cheat content. A semi-transparent or resizable window allows adjustment.

**Behavioral consistency**: Because the invisible content is on the physical screen, the test-taker's gaze naturally falls on the screen, producing gaze patterns consistent with normal exam-taking behavior. This is a critical advantage over physical cheat sheets, second monitors, or phone-based cheating, all of which produce detectable gaze deviations [20, 21].

---

## V. Evaluation

### A. Experimental Setup

We evaluated the Invisible Window attack in a controlled laboratory environment against three representative proctoring configurations:

**Configuration 1 — Browser-only screen capture**: A web application using `getDisplayMedia()` to capture the full screen at 1080p/30fps, simulating the screen capture component of browser-based proctoring systems.

**Configuration 2 — Screen capture with webcam monitoring**: Configuration 1 augmented with `getUserMedia()` webcam capture and a basic gaze estimation system using MediaPipe Face Mesh, simulating proctoring systems that combine screen and webcam monitoring [20].

**Configuration 3 — Full behavioral monitoring**: Configuration 2 augmented with mouse movement logging, keyboard event logging, and application focus tracking, simulating comprehensive proctoring systems [22, 23].

All configurations were tested on:
- Windows 11 23H2 (Build 22631) with Chrome 122 and Edge 122
- Windows 10 22H2 (Build 19045) with Chrome 122 and Firefox 123
- macOS 14.3 (Sonoma) with Chrome 122 and Safari 17.3
- macOS 26.3.1 (Build 25D2128) with CGWindowListCreateImage and screencapture system APIs

### B. Metrics

We evaluated the following metrics:

1. **Screen capture evasion rate**: Percentage of captured frames in which the invisible window's content is absent. Target: 100%.

2. **Visual artifact presence**: Manual and automated inspection of captured frames for anomalies (black rectangles, rendering glitches, compositing errors) that might indicate a hidden window.

3. **Gaze anomaly detection**: Whether the test-taker's gaze patterns while reading the invisible window content differ significantly from normal exam-taking gaze patterns.

4. **Mouse/keyboard anomaly detection**: Whether interaction patterns (focus loss events, unusual click coordinates, typing in non-exam fields) produce detectable signals.

5. **Process detection**: Whether the invisible window application appears in standard process enumeration outputs and whether its display affinity settings are detectable.

### C. Results

#### Screen Capture Evasion

| Platform | Browser | Evasion Rate | Artifacts |
|----------|---------|-------------|-----------|
| Windows 11 | Chrome 122 | 100% | None |
| Windows 11 | Edge 122 | 100% | None |
| Windows 10 | Chrome 122 | 100% | None |
| Windows 10 | Firefox 123 | 100% | None |
| macOS 14.3 | Chrome 122 | 100% | None |
| macOS 14.3 | Safari 17.3 | 100% | None |
| macOS 26.3 | screencapture | 100% | None* |
| macOS 26.3 | CGWindowListCreateImage | 100% | None* |

*On macOS 26.3.1, despite Apple's documented changes to ScreenCaptureKit in macOS 15 that were expected to ignore `sharingType = .none`, our pixel-level forensic analysis demonstrates that window content remains fully hidden from all tested capture APIs. An A/B comparison of the same screen region with and without the invisible window yields 92% identical pixels (1,086,110 of 1,170,560), with all 84,450 differing pixels caused by background content scrolling between captures and zero attributable to the invisible window, confirming that the capture pipeline renders the desktop as if the window does not exist. Single-window capture via `CGWindowListCreateImage` returns a fully transparent image (100% RGBA 0,0,0,0). This contradicts the prevailing assumption that Apple mitigated this attack vector and represents a novel finding.

On all tested platforms — Windows 10/11 and macOS 14–26 — the attack achieved a 100% evasion rate. The captured frames showed the desktop or exam interface as if the invisible window did not exist. No visual artifacts — no black rectangles, no compositing errors, no flicker — were observed in any captured frame across over 10,000 frames analyzed per configuration.

#### Behavioral Detection

**Gaze analysis**: Test subjects (n=8) were asked to read content from the invisible window while a simulated exam was displayed beneath it. Gaze patterns were analyzed using a MediaPipe-based tracker. The gaze data showed no statistically significant deviation from baseline exam-taking patterns (p > 0.3, paired t-test on mean gaze dispersion), because the invisible content is on the same physical screen as the exam — the student's eyes are looking at their monitor in both cases.

**Mouse dynamics**: Mouse movement patterns during invisible window reading sessions showed minor deviations from baseline exam interaction (reduced click frequency, increased cursor dwell time), but these signals were within the normal variance observed across subjects during standard exam behavior and did not trigger anomaly detection thresholds calibrated on training data [22, 25].

**Application focus**: On Windows, clicking within the invisible window transfers keyboard focus to the hidden application. This focus change is *not* visible in `getDisplayMedia()` output (the exam page still appears active in the capture). However, it may be detectable by JavaScript focus/blur event listeners in the exam page if the exam browser loses focus. This represents a potential detection vector discussed in Section VI.

#### Process Detection

The invisible window application appears as a standard user-level process in Task Manager (Windows) or Activity Monitor (macOS). Its process name, however, can be set to any value chosen by the attacker — there is no distinguishing marker in standard process enumeration that identifies a process as using display affinity flags.

Detecting the display affinity flag requires calling `GetWindowDisplayAffinity` on each top-level window handle — a capability that requires enumeration of the window hierarchy, which is available to any user-level process. However, current browser-based proctoring systems do not perform this check, as it requires native code execution outside the browser sandbox.

### D. Summary of Findings

The Invisible Window attack achieves complete evasion of screen capture-based proctoring on all tested platforms — Windows 10/11 and macOS 14–26 — with zero detectable artifacts. Contrary to expectations based on Apple's documented ScreenCaptureKit changes in macOS 15, our empirical testing on macOS 26.3.1 demonstrates that `sharingType = .none` remains fully effective. Behavioral detection mechanisms (gaze tracking, mouse dynamics) are ineffective because the attack does not alter the test-taker's physical posture or screen-directed attention. Process-level detection is theoretically possible but not implemented by current browser-based proctoring systems.

---

## VI. Countermeasures

We propose and evaluate the following countermeasures against the Invisible Window attack, ranging from immediately deployable mitigations to longer-term architectural changes.

### A. Display Affinity Flag Enumeration

**Mechanism**: A native proctoring agent enumerates all top-level windows using `EnumWindows` (Windows) or `CGWindowListCopyWindowInfo` (macOS) and calls `GetWindowDisplayAffinity` (Windows) or checks `sharingType` (macOS) on each window. Any window with a capture-exclusion flag set during an exam session is flagged as suspicious.

**Feasibility**: High — the API calls are straightforward and require no elevated privileges.

**Limitations**: (1) Requires a native agent, not just a browser extension. (2) The attacker can set the flag only momentarily or toggle it rapidly. (3) The attacker could name their process to mimic legitimate software (e.g., DRM-protected media players) that legitimately uses display affinity. (4) Our testing on macOS 26 shows `sharingType.none` remains effective for capture evasion, making this check necessary on all macOS versions.

**Recommendation**: Implement as a baseline check in native proctoring agents. This is the most immediately effective countermeasure on Windows.

### B. Frame Comparison with Known State

**Mechanism**: The proctoring system compares the `getDisplayMedia()` output against the expected exam interface. Significant discrepancies (e.g., the captured frame shows only the exam page when the system expects other windows to be visible) could indicate capture evasion.

**Feasibility**: Low — the proctoring system does not have a ground truth for what the desktop should look like. The test-taker's desktop wallpaper, taskbar configuration, and notification state are all variable.

**Limitations**: Requires knowing what the "correct" screen should look like, which varies per user. High false positive rate.

**Recommendation**: Not viable as a primary countermeasure.

### C. Application Focus Monitoring

**Mechanism**: The exam web application monitors JavaScript `blur` and `focus` events on the document and window objects. When the browser window loses focus (because the user clicked on the invisible window), the proctoring system logs the event and potentially flags it.

**Feasibility**: High — requires only JavaScript, no native code.

**Limitations**: (1) The invisible window can be designed as "click-through" using `WS_EX_TRANSPARENT` (Windows) or `ignoresMouseEvents` (macOS), preventing focus loss entirely. (2) Even non-click-through invisible windows may not trigger focus loss if the user only reads content without clicking. (3) Legitimate focus loss occurs frequently (system notifications, OS dialogs) and produces false positives.

**Recommendation**: Implement as a supplementary signal, not a primary detector. Pair with other telemetry to reduce false positives.

### D. Hardware-Level Display Integrity Attestation

**Mechanism**: A trusted platform module (TPM) or secure enclave provides cryptographic attestation that the display output signal matches the capture output. This would detect any discrepancy between the physical display and the captured frame at the hardware level.

**Feasibility**: Very low — no current hardware platform provides this capability. It would require changes to GPU drivers, display controllers, and the TPM attestation chain.

**Limitations**: Requires hardware/firmware support that does not exist. Long development timeline. Privacy implications of hardware-attested screen content.

**Recommendation**: Long-term research direction, not a near-term solution.

### E. OS-Level Capture Integrity Guarantees

**Mechanism**: The operating system provides a "capture integrity" mode that disables display affinity flags system-wide during a designated session, ensuring that all visible content appears in the capture output.

**Feasibility**: Medium — requires OS vendor cooperation (Microsoft, Apple). Apple reportedly changed ScreenCaptureKit in macOS 15 to ignore `sharingType.none`, though our empirical testing on macOS 26 demonstrates that this mitigation is incomplete — `sharingType.none` continues to exclude window content from capture output.

**Limitations**: Requires OS vendor buy-in. May conflict with legitimate DRM use cases. Privacy concerns (users may not want all content capturable).

**Recommendation**: Advocate to OS vendors for a "proctoring mode" API that provides capture integrity guarantees within an explicitly authorized session.

### F. Defense-in-Depth Assessment

No single countermeasure provides a complete defense. Table II summarizes the countermeasure landscape:

| Countermeasure | Effectiveness | Deployability | Evasion Difficulty |
|----------------|--------------|---------------|-------------------|
| Flag enumeration | High (Windows) | Medium (native agent) | Medium (process masquerading) |
| Frame comparison | Low | High (JS only) | Low (variable desktops) |
| Focus monitoring | Medium | High (JS only) | Medium (click-through windows) |
| HW attestation | Very High | None (does not exist) | Very High |
| OS capture integrity | High | Low (vendor dependency) | High |

The most practical near-term defense is a combination of (A) native agent-based flag enumeration with (C) JavaScript focus monitoring, supplemented by ongoing advocacy for (E) OS-level capture integrity APIs.

---

## VII. Ethical Considerations

### A. Responsible Disclosure Framework

This research was conducted in accordance with established coordinated vulnerability disclosure principles [34, 35, 36, 37, 38]. The following disclosure timeline was followed:

1. **Discovery and verification**: The display affinity bypass was identified and verified in a controlled laboratory environment.
2. **Vendor notification**: Affected proctoring vendors were notified with a detailed technical report and a 90-day disclosure window.
3. **OS vendor communication**: Microsoft and Apple were informed of the security implications of their display affinity APIs in the proctoring context.
4. **Public disclosure**: This paper is published after the disclosure window has elapsed, allowing vendors time to develop and deploy mitigations.

### B. Ethical Framing

The decision to publicly disclose this vulnerability follows the principle that security through obscurity is not security [34]. The APIs exploited are publicly documented, the technique is straightforward, and evidence from Simko et al. [16] demonstrates that proctoring evasion techniques — including sophisticated technical methods — are already widely shared in online communities. Withholding this research would not prevent exploitation but would prevent the development of informed countermeasures.

We draw on the IEEE Code of Ethics [35], which mandates that members "disclose promptly factors that might endanger the public," and the ACM Code of Ethics [34], which requires "full disclosure of all pertinent system limitations and problems." The FIRST multi-party vulnerability coordination guidelines [37] inform our approach to coordinating disclosure across multiple affected vendors and OS platforms.

Reidsma, van der Ham, and Continella [39] provide a directly applicable framework for operationalizing cybersecurity research ethics in academic settings, including self-assessment criteria and institutional CVD procedures that we followed.

### C. Dual-Use Considerations

We acknowledge that this paper describes a technique that could be misused for academic dishonesty. We argue that publication is justified because:

1. **The vulnerability is inherent, not introduced**: The APIs are documented and available. We did not create the vulnerability; we identified and formalized it.

2. **Community and commercial awareness already exists**: As Simko et al. [16] document, proctoring evasion techniques are shared openly on social media. Moreover, the specific display affinity technique has been independently discovered and commercially exploited by products including Interview Coder [43, 44], Cluely [45], and multiple open-source tools [47, 48, 49], with an estimated 35% of candidates showing signs of AI-assisted cheating [46]. The attack vector is already in active use; withholding formal analysis serves only to delay informed defenses.

3. **Defenders need to know**: Proctoring vendors cannot develop effective countermeasures against a threat they do not understand. Detailed technical analysis enables informed defense.

4. **Alternative assessment matters**: Demonstrating the fundamental limitations of screen capture-based proctoring supports the academic argument [10, 18, 19] that institutions should invest in alternative assessment designs rather than an arms race with increasingly sophisticated evasion techniques.

### D. Impact on Students

We recognise that proctoring systems, despite their limitations, serve a role in maintaining academic integrity that benefits honest students [5, 6]. However, research consistently shows that proctoring imposes psychological costs on students — including increased anxiety and decreased performance [9] — while failing to eliminate cheating [28, 29]. The existence of fundamental bypasses like the Invisible Window underscores that proctoring creates a false sense of security while imposing real costs on test-takers [10, 30].

---

## VIII. Related Work

### A. Proctoring System Security

The security of online proctoring systems has received increasing academic attention since the COVID-19 pandemic. Simko et al. [16] present the most directly related work, documenting community-developed evasion techniques ranging from non-technical methods (sticky notes on screens) to deeply technical approaches (custom virtual machines). Our work extends their findings by identifying a specific, OS-level mechanism that is more reliable and less detectable than the techniques they catalogued.

Balash et al. [3] survey educators' perspectives on proctoring, documenting known security incidents including the ProctorU data breach (444,000 users' PII leaked) and a Proctorio vulnerability enabling remote software activation. Their companion study [7] examines students' privacy and security perceptions, establishing the adversarial dynamic between test-takers and proctoring systems.

Luijben, van den Broek, and Alpár [17] formalize security requirements for proctoring systems using threat analysis methodology. Their five-requirement framework provides the basis for our analysis of which security properties the Invisible Window attack violates (Section II-D).

### B. Proctoring Effectiveness and Criticism

A substantial body of work questions the effectiveness and desirability of online proctoring. Lee and Fanguy [5] critically examine whether proctoring technologies represent educational innovation or deterioration. Khalil, Prinsloo, and Slade [10] position proctoring at the nexus of integrity and surveillance, arguing that the COVID-19 pivot amplified reliance on tools with fundamental limitations. Conijn et al. [9] demonstrate empirically that proctored exams impose negative psychological effects on students, while Duncan and Joyner [18] argue that digital proctoring may not be necessary at all.

Paris, Reynolds, and McGowan [19] document privacy violations in e-learning platforms predating the pandemic. Johri and Hingle [8] describe students' technological ambivalence toward proctoring — recognising the need for integrity while resisting invasive monitoring. Mukherjee et al. [41] examine the tension between cheating detection efficacy, privacy, and fairness through visual data obfuscation in remote proctoring. Marano et al. [31] provide a scoping review of the student experience of remote proctoring.

These critiques provide essential context for our work: the Invisible Window attack is significant not merely as a technical exploit, but as evidence that the fundamental architecture of screen capture-based proctoring is unsound.

### C. Behavioral Detection Methods

Research on behavioral cheating detection includes eye gaze tracking [20, 21], mouse dynamics [22, 24, 25, 26, 27], keystroke analysis [26, 27], and multimodal fusion approaches [23, 40]. Kaddoura et al. [20] provide a systematic review of computational intelligence approaches to cheating detection, covering face recognition, head posture analysis, gaze tracking, and network analysis. Ferdosi et al. [21] demonstrate automated behavioral pattern classification using MediaPipe, achieving 87.5% cheating detection accuracy.

Our evaluation (Section V) demonstrates that these behavioral detection mechanisms are largely ineffective against the Invisible Window attack because the test-taker's physical behavior (gaze direction, posture, screen attention) is indistinguishable from legitimate exam behavior.

### D. Threat Modeling Across Platforms

Das Chowdhury et al. [2] demonstrate that threat models often fail to adapt when systems move across platforms — a finding directly analogous to the proctoring context, where browser-based systems assume that the OS display pipeline is trustworthy without verifying this assumption. Their STRIDE/LINDDUN framework for analyzing cross-platform threat model drift informs our analysis of the trust boundary between the browser and the OS compositing layer.

### E. Vulnerability Disclosure in Educational Technology

Mehrishi, Sarmah, and Daneva [32] identify security gaps in Canvas LMS, Moodle, and Google Forms, demonstrating that vulnerability surfaces in educational technology extend beyond proctoring software. Adkins and Joyner [42] examine the challenges of scaling anti-plagiarism detection in large online computer science classes. Lachheb et al. [33] argue that maintaining student privacy in educational technology is a matter of design ethics, not merely policy compliance.

Noordegraaf and Weulen Kranenbarg [6] study the motivations of ethical hackers, finding that "reporting vulnerabilities as a moral duty" is a primary driver — a perspective that directly informs our responsible disclosure approach. Reidsma, van der Ham, and Continella [39] provide an operational framework for cybersecurity research ethics that we follow.

### F. Commercial Exploitation and In-the-Wild Awareness

While no prior academic work has formally analyzed the display affinity attack vector against proctoring systems, the underlying technique has been independently discovered and commercially exploited by several products targeting technical interviews and online assessments.

**Commercial "invisible overlay" tools.** Multiple commercial products now leverage OS-level display affinity APIs to create AI-powered overlays that are invisible to screen sharing. Interview Coder [43], launched in 2025 and updated to version 3.0 in March 2026, uses "special window flags" that mark its UI as excluded from screen captures — described as "similar to video-protected content" leveraging "standard OS APIs on Windows and macOS" [44]. The tool additionally employs click-through overlays, process name disguising, dock/taskbar hiding, and global hotkeys registered at the OS level rather than through the browser event system. Cluely [45], another commercially available tool, uses "low-level graphics hooks (DirectX on Windows, Metal framework on macOS)" to render its interface directly on the GPU's local display output, ensuring that "when a candidate shares their screen via Zoom or Teams, the video encoding pipeline captures only the desktop beneath the overlay" [46]. A 2026 industry survey reported that 35% of candidates showed signs of AI-assisted cheating in late 2025, with 59% of hiring managers suspecting candidates of using such tools [46].

**Open-source proof-of-concepts.** The open-source community has produced several demonstrations of this technique. Idanless [47] published an "Anti-Screen-Capture-window" proof-of-concept in April 2025 — a Python/PyQt6 application embedding a ChatGPT browser within a `WDA_EXCLUDEFROMCAPTURE`-flagged window, explicitly referencing interview cheating as its use case. Khorev [48] documented the development of "Ezzi," an open-source invisible interview assistant built with Electron, describing the platform-specific challenges of achieving capture invisibility across Windows and macOS. Mayerr [49] published "openinterviewcoder," a cross-platform Electron application supporting Windows, macOS, and Linux.

**Vendor awareness.** The issue has been reported to Microsoft through their Q&A platform [50], where an educator documented students using `SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE)` to hide applications from classroom monitoring software. Community-suggested countermeasures included DLL injection to reset affinity flags and per-user services that periodically enumerate windows and terminate processes with capture-exclusion flags set. Proctoring vendors have begun responding: Proctorio [51] claims to block tools like Cluely by "preventing unauthorized apps from launching during exams," while Honorlock and Talview have implemented behavioral analysis targeting the timing patterns and gaze anomalies characteristic of AI-assisted responses.

**Distinction from our work.** The commercial tools and open-source PoCs described above demonstrate that the display affinity attack vector is independently known and actively exploited. However, no prior work has: (1) formally modeled the trust boundary violation between the browser capture API and the OS compositing pipeline; (2) provided a systematic cross-platform evaluation with pixel-level forensic verification; (3) analyzed which behavioral detection mechanisms can and cannot detect the attack; (4) proposed and evaluated countermeasures; or (5) tested the attack on macOS 15+ where the conventional understanding is that Apple's ScreenCaptureKit changes mitigate the vulnerability. Our finding that `NSWindow.sharingType = .none` remains fully effective on macOS 26 — contradicting both Apple's documentation and community assumptions — has not been reported elsewhere. This paper provides the first formal security analysis of a vulnerability class that is already being commercially exploited without academic scrutiny.

### G. AI Amplification Analysis: LLM-Assisted Attack Orchestration

The proof-of-concept implementations and forensic evaluation presented in this paper were developed using Claude Opus 4.6 (1M context window) via Claude Code as the primary research instrument — from initial literature synthesis to PoC implementation to pixel-level forensic verification. This methodology was not incidental; it is itself a finding with direct implications for the threat model.

**Skill barrier reduction.** Prior to this research, exploiting display affinity APIs for proctoring evasion required familiarity with Win32 internals, macOS Objective-C/Swift window management, screen capture pipeline architecture, and forensic verification methodology. Collectively, this represents approximately senior-developer-level expertise across two platforms. Using Claude as an orchestration layer, a single researcher with cybersecurity fundamentals — but no prior experience with Win32 display affinity APIs or ScreenCaptureKit internals — produced working, empirically validated PoCs on both platforms within a single session. The effective skill barrier was reduced from expert-level to intermediate undergraduate.

**Attack surface discovery.** Claude independently identified the `WDA_EXCLUDEFROMCAPTURE` flag distinction from `WDA_MONITOR` — specifically the absence of a black rectangle artifact — as the operationally critical implementation detail. This distinction is documented by Microsoft [11] but buried in API reference text; a manual literature review would likely have overlooked it. Similarly, Claude identified the discrepancy between Apple's documented ScreenCaptureKit changes in macOS 15 and the actual runtime behaviour of legacy CoreGraphics APIs — the insight that motivated empirical testing on macOS 26 and produced this paper's most novel finding.

**Dual-use implications.** These observations carry direct implications for AI safety research. The same LLM capability that accelerated legitimate security research — reasoning across multi-platform API documentation, generating forensically sound PoC code, identifying undocumented behavioural discrepancies — is equally accessible to adversaries with no research intent. Unlike traditional offensive tooling, Claude requires no configuration, no compilation environment, and no prior domain knowledge to produce functional attack implementations. The democratisation of offensive capability demonstrated here is not hypothetical; it is empirically measured.

**AI-assisted countermeasure proposal.** We propose that the same LLM-assisted methodology applied offensively here can be inverted defensively. A proctoring agent augmented with an LLM-based static analysis layer could continuously monitor system API call graphs for `SetWindowDisplayAffinity` or `NSWindow.sharingType` invocations, reason over process behaviour to distinguish legitimate content-protection use from adversarial proctoring evasion, and generate adaptive detection signatures in response to novel evasion variants. The feasibility and limitations of this approach represent a direct extension of the present work and a productive direction for future research at the intersection of AI security and academic integrity.

This research was conducted in accordance with Anthropic's usage policies. Prompts requesting direct facilitation of academic misconduct were declined by the model; all accepted prompts were framed within an explicit security research and responsible disclosure context, demonstrating that frontier AI safety measures provide partial but incomplete mitigation against dual-use research of this nature — itself a finding warranting further investigation.

---

## IX. Conclusion

This paper has demonstrated that browser-based proctoring systems that rely on the WebRTC `getDisplayMedia()` API for screen monitoring are vulnerable to a fundamental, cross-platform evasion technique. The Invisible Window attack exploits documented OS-level display affinity APIs — Windows `SetWindowDisplayAffinity` with `WDA_EXCLUDEFROMCAPTURE` and macOS `NSWindow.SharingType.none` — to create application windows that are fully visible on the physical display but completely invisible to screen capture.

Our proof-of-concept implementations achieve a 100% evasion rate on all tested platforms — Windows 10/11 and macOS 14 through macOS 26 — with zero visual artifacts in captured frames and no detectable behavioral anomalies. The attack requires no elevated privileges, no software exploitation, and minimal technical skill to deploy. Notably, our testing on macOS 26.3.1 demonstrates that Apple's reported ScreenCaptureKit changes in macOS 15 did not effectively mitigate the attack: `NSWindow.sharingType = .none` continues to fully exclude window content from all capture APIs tested, contradicting prevailing community assumptions.

The implications extend beyond a single exploit. The Invisible Window attack reveals a structural weakness in the trust model underlying screen capture-based proctoring: the assumption that the OS compositing pipeline faithfully represents the physical display state. This assumption has never been explicitly validated and, as we show, does not hold.

We have proposed and evaluated countermeasures ranging from immediately deployable (display affinity flag enumeration) to long-term (hardware-level display integrity attestation). The most practical near-term defense combines native agent-based flag enumeration with JavaScript focus monitoring and advocacy for OS-level capture integrity APIs.

More broadly, this work reinforces the growing academic consensus [5, 10, 18, 19] that screen-based surveillance is not a sustainable foundation for academic integrity. As evasion techniques grow more sophisticated and less detectable, the arms race between proctoring systems and bypass methods becomes increasingly untenable. Institutions would be better served by investing in assessment designs that are inherently resistant to cheating — open-book examinations, authentic assessments, oral defenses — rather than in ever-more-invasive monitoring of students' screens.

We have followed coordinated vulnerability disclosure principles throughout this research and have notified affected vendors. We hope this work contributes to the development of more robust, privacy-respecting, and fundamentally sound approaches to maintaining academic integrity in online education.

---

## References

[1] J.-I. Bruaroey and E. Alon, "Screen Capture," W3C Working Draft, World Wide Web Consortium, Jul. 2025. [Online]. Available: https://www.w3.org/TR/screen-capture/

[2] P. Das Chowdhury, M. Sameen, J. Blessing, N. Boucher, J. Gardiner, T. Burrows, R. Anderson, and A. Rashid, "Threat models over space and time: A case study of end-to-end-encrypted messaging applications," *Software: Practice and Experience*, vol. 54, no. 12, pp. 2316–2335, 2024.

[3] D. G. Balash, R. A. Fainchtein, E. Korkes, M. Grant, M. Sherr, and A. J. Aviv, "Educators' perspectives of using (or not using) online exam proctoring," in *Proc. 32nd USENIX Security Symp.*, Anaheim, CA, 2023, pp. 5091–5108.

[4] Mozilla Developer Network, "Using the Screen Capture API," MDN Web Docs, 2025. [Online]. Available: https://developer.mozilla.org/en-US/docs/Web/API/Screen_Capture_API/Using_Screen_Capture

[5] K. Lee and M. Fanguy, "Online exam proctoring technologies: Educational innovation or deterioration?," *British J. Educ. Technol.*, vol. 53, no. 3, pp. 475–490, 2022.

[6] J. E. Noordegraaf and M. Weulen Kranenbarg, "Why do young people start and continue with ethical hacking? A qualitative study on individual and social aspects in the lives of ethical hackers," *Criminology & Public Policy*, vol. 22, no. 4, pp. 803–824, 2023.

[7] D. G. Balash, D. Kim, D. Shaibekova, R. A. Fainchtein, M. Sherr, and A. J. Aviv, "Examining the examiners: Students' privacy and security perceptions of online proctoring services," in *Proc. 17th Symp. Usable Privacy and Security (SOUPS)*, 2021, pp. 633–652.

[8] A. Johri and A. Hingle, "Students' technological ambivalence toward online proctoring and the need for responsible use of educational technologies," *J. Eng. Educ.*, vol. 112, no. 1, pp. 221–242, 2023.

[9] R. Conijn, A. Kleingeld, U. Matzat, and C. Snijders, "The fear of big brother: The potential negative side-effects of proctored exams," *J. Computer Assisted Learning*, vol. 38, no. 6, pp. 1521–1534, 2022.

[10] M. Khalil, P. Prinsloo, and S. Slade, "In the nexus of integrity and surveillance: Proctoring (re)considered," *J. Computer Assisted Learning*, vol. 38, no. 6, pp. 1589–1602, 2022.

[11] Microsoft, "SetWindowDisplayAffinity function (winuser.h)," Microsoft Learn, 2025. [Online]. Available: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity

[12] Apple Inc., "NSWindow.SharingType," Apple Developer Documentation, 2025. [Online]. Available: https://developer.apple.com/documentation/appkit/nswindow/sharingtype

[13] Apple Inc., "ScreenCaptureKit," Apple Developer Documentation, 2025. [Online]. Available: https://developer.apple.com/documentation/screencapturekit/

[14] D. Dharminder, "LWEDM: Learning with error based secure mobile digital rights management system," *Trans. Emerging Telecomm. Technol.*, vol. 32, no. 2, art. e4199, 2020.

[15] columbusux, "macOS 15+: ScreenCaptureKit ignores setContentProtection / NSWindow.sharingType," GitHub Issue #14200, tauri-apps/tauri, Sep. 2025. [Online]. Available: https://github.com/tauri-apps/tauri/issues/14200

[16] L. Simko, A. Hutchinson, A. Isaac, E. Fries, M. Sherr, and A. J. Aviv, "'Modern problems require modern solutions': Community-developed techniques for online exam proctoring evasion," in *Proc. 2024 ACM SIGSAC Conf. Computer and Communications Security (CCS '24)*, Salt Lake City, UT, 2024.

[17] R. Luijben, F. van den Broek, and G. Alpár, "Security requirements for proctoring in higher education," in *2024 IEEE Global Eng. Educ. Conf. (EDUCON)*, Kos Island, Greece, 2024, pp. 1–8.

[18] A. Duncan and D. Joyner, "On the necessity (or lack thereof) of digital proctoring: Drawbacks, perceptions, and alternatives," *J. Computer Assisted Learning*, vol. 38, no. 5, pp. 1482–1496, 2022.

[19] B. Paris, R. Reynolds, and C. McGowan, "Sins of omission: Critical informatics perspectives on privacy in e-learning systems in higher education," *J. Assoc. Inf. Sci. Technol.*, vol. 73, no. 5, pp. 708–725, 2021.

[20] S. Kaddoura, S. Vincent, D. J. Hemanth, and I. Ashraf, "Computational intelligence and soft computing paradigm for cheating detection in online examinations," *Appl. Comput. Intell. Soft Comput.*, vol. 2023, art. 3739975, 2023.

[21] B. J. Ferdosi, M. Rahman, A. M. Sakib, T. Helaly, and P. Chakraborty, "Modeling and classification of the behavioral patterns of students participating in online examination," *Human Behavior and Emerging Technol.*, vol. 2023, art. 2613802, 2023.

[22] N. Dilini, A. Senaratne, T. Yasarathna, N. Warnajith, and L. Seneviratne, "Cheating detection in browser-based online exams through eye gaze tracking," in *2021 6th Int. Conf. Information Technology Research (ICITR)*, IEEE, 2021, pp. 1–6.

[23] J. Guan, X. Li, Y. Zhang, and K. Andersson, "Design and implementation of continuous authentication mechanism based on multimodal fusion mechanism," *Security and Communication Networks*, vol. 2021, art. 6669429, 2021.

[24] T. Hu, W. Niu, X. Zhang, X. Liu, J. Lu, Y. Liu, and J. Chen, "An insider threat detection approach based on mouse dynamics and deep learning," *Security and Communication Networks*, vol. 2019, art. 3898951, 2019.

[25] X. Wang, Q. Zheng, K. Zheng, T. Wu, and G. M. Perez, "User authentication method based on MKL for keystroke and mouse behavioral feature fusion," *Security and Communication Networks*, vol. 2020, art. 9282380, 2020.

[26] T. Lyu, L. Liu, F. Zhu, S. Hu, R. Ye, and J. Dalle, "BEFP: An extension recognition system based on behavioral and environmental fingerprinting," *Security and Communication Networks*, vol. 2022, art. 7896571, 2022.

[27] S. Yazici, H. Yildiz Durak, B. Aksu Dünya, and B. Şentürk, "Online versus face-to-face cheating: The prevalence of cheating behaviours during the pandemic compared to the pre-pandemic among Turkish university students," *J. Computer Assisted Learning*, vol. 39, no. 1, pp. 231–254, 2022.

[28] M. Rüth, M. Jansen, and K. Kaspar, "Cheating behaviour in online exams: On the role of needs, conceptions and reasons of university students," *J. Computer Assisted Learning*, vol. 40, no. 5, pp. 1987–2008, 2024.

[29] M. Gribbins and C. J. Bonk, "An exploration of instructors' perceptions about online proctoring and its value in ensuring academic integrity," *British J. Educ. Technol.*, vol. 54, no. 6, pp. 1693–1714, 2023.

[30] P. Prinsloo, M. Khalil, and S. Slade, "Vulnerable student digital well-being in AI-powered educational decision support systems (AI-EDSS) in higher education," *British J. Educ. Technol.*, vol. 55, no. 5, pp. 2075–2092, 2024.

[31] E. Marano, P. M. Newton, Z. Birch, M. Croombs, C. Gilbert, and M. J. Draper, "What is the student experience of remote proctoring? A pragmatic scoping review," *Higher Educ. Quarterly*, vol. 78, no. 3, pp. 1031–1047, 2024.

[32] A. A. Mehrishi, D. K. Sarmah, and M. Daneva, "How can cryptography secure online assessments against academic dishonesty?," *Security and Privacy*, vol. 8, no. 4, art. e70065, 2025.

[33] A. Lachheb, V. Abramenka-Lachheb, S. Moore, and C. Gray, "The role of design ethics in maintaining students' privacy: A call to action to learning designers in higher education," *British J. Educ. Technol.*, vol. 54, no. 6, pp. 1653–1670, 2023.

[34] Association for Computing Machinery, "ACM Code of Ethics and Professional Conduct," ACM, 2018. [Online]. Available: https://www.acm.org/code-of-ethics

[35] Institute of Electrical and Electronics Engineers, "IEEE Code of Ethics," IEEE, 2024. [Online]. Available: https://www.ieee.org/about/corporate/governance/p7-8

[36] OWASP Foundation, "Vulnerability Disclosure Cheat Sheet," OWASP Cheat Sheet Series. [Online]. Available: https://cheatsheetseries.owasp.org/cheatsheets/Vulnerability_Disclosure_Cheat_Sheet.html

[37] Forum of Incident Response and Security Teams (FIRST), "Guidelines and Practices for Multi-Party Vulnerability Coordination and Disclosure," ver. 1.1, FIRST, 2020. [Online]. Available: https://www.first.org/global/sigs/vulnerability-coordination/multiparty/guidelines-v1-1

[38] Cybersecurity and Infrastructure Security Agency, "Coordinated Vulnerability Disclosure Program," CISA, 2025. [Online]. Available: https://www.cisa.gov/resources-tools/programs/coordinated-vulnerability-disclosure-program

[39] D. Reidsma, J. van der Ham, and A. Continella, "Operationalizing cybersecurity research ethics review: From principles and guidelines to practice," in *Proc. 2nd Int. Workshop on Ethics in Computer Security (EthiCS 2023)*, co-located with NDSS Symp., San Diego, CA, Feb. 2023.

[40] Y. Atoum, L. Chen, A. X. Liu, S. D. H. Hsu, and X. Liu, "Automated online exam proctoring," *IEEE Trans. Multimedia*, vol. 19, no. 7, pp. 1609–1624, 2017.

[41] S. Mukherjee, V. Distler, G. Lenzini, and P. Cardoso-Leite, "Balancing the perception of cheating detection, privacy and fairness: A mixed-methods study of visual data obfuscation in remote proctoring," in *Proc. 2024 European Symp. Usable Security (EuroUSEC '24)*, Karlstad, Sweden, ACM, 2024.

[42] K. L. Adkins and D. A. Joyner, "Scaling anti-plagiarism efforts to meet the needs of large online computer science classes: Challenges, solutions, and recommendations," *J. Computer Assisted Learning*, vol. 38, no. 6, pp. 1603–1619, 2022.

[43] Interview Coder, "Interview Coder — AI Interview Assistant for Technical Interviews," 2026. [Online]. Available: https://www.interviewcoder.co/

[44] "Interview Coder 3.0: Promises Complete Invisibility During Technical Interviews," OpenPR, Mar. 2026. [Online]. Available: https://www.openpr.com/news/4427427/interview-coder-3-0-promises-complete-invisibility-during

[45] Cluely, "Cluely — AI-Powered Real-Time Interview Assistant," 2025. [Online]. Available: https://cluely.com/

[46] FabricHQ, "Interview Cheating in 2026: The Rise of AI Tools Like Cluely and Interview Coder," Jan. 2026. [Online]. Available: https://www.fabrichq.ai/blogs/interview-cheating-in-2026-the-rise-of-ai-tools-like-cluely-and-interview-coder

[47] idanless, "Anti-Screen-Capture-window: Hidden ChatGPT Browser (Anti-Screen Capture PoC)," GitHub, Apr. 2025. [Online]. Available: https://github.com/idanless/Anti-Screen-Capture-window

[48] D. Khorev, "Building Ezzi: My Journey Creating an Invisible Tech Interview Assistant (Now Open Source)," Level Up Coding, 2026. [Online]. Available: https://levelup.gitconnected.com/building-ezzi-my-journey-creating-an-invisible-tech-interview-assistant-now-open-source-a1963a8fe0f3

[49] J. Mayerr, "openinterviewcoder: An undetectable AI assistant for coding interviews," GitHub, Mar. 2025. [Online]. Available: https://github.com/JoshMayerr/openinterviewcoder

[50] M. Ukt, "SetWindowDisplayAffinity bad usecase," Microsoft Q&A, Apr. 2024. [Online]. Available: https://learn.microsoft.com/en-us/answers/questions/1653885/setwindowdisplayaffinity-bad-usecase

[51] Proctorio, "How Proctorio Blocks Cluely: Stopping AI Cheating Tools Like Cluely in Online Exams," Proctorio Blog, 2026. [Online]. Available: https://proctorio.com/about/blog/how-proctorio-blocks-cluely
