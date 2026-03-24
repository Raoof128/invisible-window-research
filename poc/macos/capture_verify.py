#!/usr/bin/env python3
"""
capture_verify.py — Verification tool for the Invisible Window PoC
Author: Mohammad Raouf Abedini <mohammadraouf.abedini@students.mq.edu.au>

Takes a screenshot via macOS screencapture (which uses the OS capture pipeline),
then analyzes whether the invisible window appears in the captured output.

Usage:
    python3 capture_verify.py                    # Take screenshot + analyze
    python3 capture_verify.py --compare file.png # Compare existing screenshot
    python3 capture_verify.py --auto             # Automated test: launch PoC, capture, analyze
"""

import subprocess
import sys
import os
import time
import json
from pathlib import Path
from datetime import datetime

OUTPUT_DIR = Path("/tmp/invisible_window_eval")
OUTPUT_DIR.mkdir(exist_ok=True)


def take_screenshot(filename: str = None) -> str:
    """Take a screenshot using macOS screencapture command."""
    if filename is None:
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = str(OUTPUT_DIR / f"capture_{ts}.png")

    result = subprocess.run(
        ["screencapture", "-x", filename],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print(f"[ERROR] screencapture failed: {result.stderr}")
        sys.exit(1)

    size = os.path.getsize(filename)
    print(f"[CAPTURE] Screenshot saved: {filename} ({size:,} bytes)")
    return filename


def get_window_list() -> list:
    """Get list of all windows using CGWindowListCopyWindowInfo via Python."""
    try:
        import Quartz
        window_list = Quartz.CGWindowListCopyWindowInfo(
            Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID
        )
        return list(window_list) if window_list else []
    except ImportError:
        # Fallback: use system_profiler or osascript
        result = subprocess.run(
            ["osascript", "-e",
             'tell application "System Events" to get name of every window of every process'],
            capture_output=True, text=True
        )
        return [{"fallback": result.stdout.strip()}]


def find_invisible_window() -> dict:
    """Check if the invisible window process is running and get its properties."""
    result = subprocess.run(
        ["pgrep", "-f", "invisible_window"],
        capture_output=True, text=True
    )
    pid = result.stdout.strip()

    info = {
        "process_running": bool(pid),
        "pid": pid if pid else None,
        "timestamp": datetime.now().isoformat(),
        "macos_version": subprocess.run(
            ["sw_vers", "-productVersion"],
            capture_output=True, text=True
        ).stdout.strip()
    }

    if pid:
        # Check window properties via Quartz if available
        try:
            import Quartz
            windows = Quartz.CGWindowListCopyWindowInfo(
                Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID
            )
            for w in windows:
                owner_pid = w.get("kCGWindowOwnerPID", 0)
                if str(owner_pid) == pid:
                    info["window_found"] = True
                    info["window_name"] = w.get("kCGWindowName", "")
                    info["window_layer"] = w.get("kCGWindowLayer", 0)
                    info["window_bounds"] = dict(w.get("kCGWindowBounds", {}))
                    info["sharing_state"] = w.get("kCGWindowSharingState", -1)
                    # kCGWindowSharingState: 0 = none, 1 = readOnly, 2 = readWrite
                    sharing_map = {0: "none (INVISIBLE)", 1: "readOnly", 2: "readWrite"}
                    info["sharing_type_label"] = sharing_map.get(
                        info["sharing_state"], f"unknown({info['sharing_state']})"
                    )
                    break
            else:
                info["window_found"] = False
        except ImportError:
            info["quartz_available"] = False

    return info


def capture_with_cgimage() -> str:
    """Take a screenshot using CGImage (Quartz) directly — tests ScreenCaptureKit path."""
    try:
        import Quartz
        from Quartz import CGWindowListCreateImage, kCGWindowListOptionOnScreenOnly
        from Quartz import CGRectInfinite, kCGWindowImageDefault, kCGNullWindowID

        image = CGWindowListCreateImage(
            CGRectInfinite,
            kCGWindowListOptionOnScreenOnly,
            kCGNullWindowID,
            kCGWindowImageDefault
        )

        if image is None:
            print("[ERROR] CGWindowListCreateImage returned None")
            return None

        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = str(OUTPUT_DIR / f"cg_capture_{ts}.png")

        # Save using CGImageDestination
        from Quartz import CGImageDestinationCreateWithURL, CGImageDestinationAddImage
        from Quartz import CGImageDestinationFinalize
        import CoreFoundation

        url = CoreFoundation.CFURLCreateWithFileSystemPath(
            None, filename, CoreFoundation.kCFURLPOSIXPathStyle, False
        )
        dest = CGImageDestinationCreateWithURL(url, "public.png", 1, None)
        CGImageDestinationAddImage(dest, image, None)
        CGImageDestinationFinalize(dest)

        size = os.path.getsize(filename)
        print(f"[CG_CAPTURE] CoreGraphics screenshot saved: {filename} ({size:,} bytes)")
        return filename

    except ImportError:
        print("[WARN] Quartz/CoreGraphics not available for direct capture test")
        return None
    except Exception as e:
        print(f"[ERROR] CGImage capture failed: {e}")
        return None


def run_evaluation():
    """Run the full evaluation: detect window, capture via multiple methods, report."""
    print("=" * 64)
    print("  INVISIBLE WINDOW — Capture Verification Tool")
    print("  Evaluating display affinity bypass on this system")
    print("=" * 64)
    print()

    # Step 1: Check if invisible window is running
    print("[1/4] Checking for invisible window process...")
    window_info = find_invisible_window()

    if not window_info["process_running"]:
        print("[WARN] Invisible window process not detected.")
        print("       Launch it first: swift invisible_window.swift")
        print("       Continuing with capture tests anyway...\n")
    else:
        print(f"  PID: {window_info['pid']}")
        if window_info.get("window_found"):
            print(f"  Window: {window_info.get('window_name', 'N/A')}")
            print(f"  Sharing state: {window_info.get('sharing_type_label', 'N/A')}")
            print(f"  Bounds: {window_info.get('window_bounds', 'N/A')}")
        print()

    # Step 2: Capture via screencapture command
    print("[2/4] Taking screenshot via screencapture (system utility)...")
    sc_file = take_screenshot()
    print()

    # Step 3: Capture via CoreGraphics directly
    print("[3/4] Taking screenshot via CoreGraphics (CGWindowListCreateImage)...")
    cg_file = capture_with_cgimage()
    print()

    # Step 4: Generate report
    print("[4/4] Generating evaluation report...")
    report = {
        "timestamp": datetime.now().isoformat(),
        "macos_version": window_info.get("macos_version", "unknown"),
        "invisible_window": window_info,
        "captures": {
            "screencapture": sc_file,
            "coregraphics": cg_file,
        },
        "instructions": (
            "Compare the screenshot files with your physical display. "
            "If the invisible window appears on your monitor but NOT in the "
            "screenshots, the display affinity bypass is working."
        )
    }

    report_file = str(OUTPUT_DIR / "evaluation_report.json")
    with open(report_file, "w") as f:
        json.dump(report, f, indent=2)
    print(f"  Report saved: {report_file}")

    # Print summary
    print()
    print("=" * 64)
    print("  RESULTS")
    print("=" * 64)
    print(f"  macOS version:         {report['macos_version']}")
    print(f"  PoC process running:   {window_info['process_running']}")
    if window_info.get("window_found"):
        print(f"  Window sharing state:  {window_info.get('sharing_type_label', 'N/A')}")
    print(f"  screencapture output:  {sc_file}")
    if cg_file:
        print(f"  CoreGraphics output:   {cg_file}")
    print()
    print("  MANUAL VERIFICATION REQUIRED:")
    print("  1. Look at your physical screen — is the green-text window visible?")
    print("  2. Open the screenshot file(s) above")
    print("  3. Is the green-text window present in the screenshot?")
    print()
    print("  If YES on screen, NO in screenshot → BYPASS CONFIRMED")
    print("  If YES on screen, YES in screenshot → BYPASS FAILED (expected on macOS 15+)")
    print("=" * 64)

    # Open screenshots for visual comparison
    if sc_file:
        subprocess.run(["open", sc_file])
    if cg_file:
        subprocess.run(["open", cg_file])


def main():
    args = sys.argv[1:]

    if "--help" in args or "-h" in args:
        print(__doc__)
        sys.exit(0)

    if "--auto" in args:
        # Launch PoC, wait, then test
        print("[AUTO] Launching invisible_window.swift in background...")
        script_dir = Path(__file__).parent
        swift_file = script_dir / "invisible_window.swift"
        if not swift_file.exists():
            print(f"[ERROR] {swift_file} not found")
            sys.exit(1)
        proc = subprocess.Popen(
            ["swift", str(swift_file)],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        print(f"[AUTO] PoC launched (PID: {proc.pid}), waiting 3 seconds...")
        time.sleep(3)
        run_evaluation()
        print(f"\n[AUTO] Terminating PoC (PID: {proc.pid})...")
        proc.terminate()
    elif "--compare" in args:
        idx = args.index("--compare")
        if idx + 1 < len(args):
            filepath = args[idx + 1]
            print(f"[COMPARE] Analyzing existing screenshot: {filepath}")
            if not os.path.exists(filepath):
                print(f"[ERROR] File not found: {filepath}")
                sys.exit(1)
            subprocess.run(["open", filepath])
            print("[COMPARE] File opened. Visually check if the invisible window appears.")
        else:
            print("[ERROR] --compare requires a file path")
            sys.exit(1)
    else:
        run_evaluation()


if __name__ == "__main__":
    main()
