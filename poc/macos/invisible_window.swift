// invisible_window.swift
// The Invisible Window — macOS Proof of Concept
// Author: Mohammad Raouf Abedini <mohammadraouf.abedini@students.mq.edu.au>
//
// Demonstrates that NSWindow.sharingType = .none creates a window
// visible on the physical display but invisible to screen capture APIs.
//
// Usage: swift invisible_window.swift [--content "custom text"] [--opacity 0.95]
//
// This is a research tool for authorized security testing only.

import AppKit
import Foundation

// MARK: - Configuration

struct Config {
    var content: String = """
    ╔══════════════════════════════════════════════════════╗
    ║           THE INVISIBLE WINDOW — PoC                ║
    ║                                                     ║
    ║  This window is VISIBLE on your physical display    ║
    ║  but INVISIBLE to screen capture APIs.              ║
    ║                                                     ║
    ║  If you take a screenshot or share your screen,     ║
    ║  this window will NOT appear in the output.         ║
    ║                                                     ║
    ║  Proof-of-Concept for:                              ║
    ║  "The Invisible Window: Exploiting OS-Level         ║
    ║   Display Affinity to Bypass WebRTC Proctoring"     ║
    ║                                                     ║
    ║  Author: Mohammad Raouf Abedini                     ║
    ║  Macquarie University, Sydney                       ║
    ║                                                     ║
    ║  sharingType = .none applied ✓                      ║
    ╚══════════════════════════════════════════════════════╝
    """
    var opacity: Double = 0.95
    var width: CGFloat = 620
    var height: CGFloat = 440
    var fontSize: CGFloat = 14
}

func parseArgs() -> Config {
    var config = Config()
    let args = CommandLine.arguments
    var i = 1
    while i < args.count {
        switch args[i] {
        case "--content":
            if i + 1 < args.count {
                config.content = args[i + 1]
                i += 2
            } else { i += 1 }
        case "--opacity":
            if i + 1 < args.count, let val = Double(args[i + 1]) {
                config.opacity = max(0.1, min(1.0, val))
                i += 2
            } else { i += 1 }
        case "--width":
            if i + 1 < args.count, let val = Double(args[i + 1]) {
                config.width = CGFloat(val)
                i += 2
            } else { i += 1 }
        case "--height":
            if i + 1 < args.count, let val = Double(args[i + 1]) {
                config.height = CGFloat(val)
                i += 2
            } else { i += 1 }
        case "--help":
            print("""
            Usage: invisible_window [OPTIONS]
              --content "text"   Custom window content
              --opacity 0.95     Window opacity (0.1-1.0)
              --width 620        Window width
              --height 440       Window height
              --help             Show this help
            """)
            exit(0)
        default:
            i += 1
        }
    }
    return config
}

// MARK: - Application Delegate

class InvisibleWindowDelegate: NSObject, NSApplicationDelegate {
    let config: Config
    var window: NSWindow?
    var statusItem: NSStatusItem?

    init(config: Config) {
        self.config = config
        super.init()
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        createInvisibleWindow()
        createStatusBarItem()
        printStatus()
    }

    func createInvisibleWindow() {
        // Calculate center position
        guard let screen = NSScreen.main else {
            print("[ERROR] No main screen found")
            exit(1)
        }
        let screenFrame = screen.visibleFrame
        let x = screenFrame.midX - config.width / 2
        let y = screenFrame.midY - config.height / 2

        let rect = NSRect(x: x, y: y, width: config.width, height: config.height)

        // Create the window
        let window = NSWindow(
            contentRect: rect,
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )

        // === THE KEY LINE: Exclude from screen capture ===
        window.sharingType = .none
        // ================================================

        window.title = "Invisible Window (sharingType = .none)"
        window.level = .floating          // Stay above other windows
        window.alphaValue = CGFloat(config.opacity)
        window.isReleasedWhenClosed = false
        window.backgroundColor = NSColor(red: 0.1, green: 0.1, blue: 0.15, alpha: 1.0)

        // Create content view with text
        let textView = NSTextView(frame: NSRect(x: 0, y: 0, width: config.width, height: config.height))
        textView.isEditable = false
        textView.isSelectable = true
        textView.backgroundColor = NSColor(red: 0.1, green: 0.1, blue: 0.15, alpha: 1.0)
        textView.textColor = NSColor(red: 0.0, green: 1.0, blue: 0.4, alpha: 1.0) // Green terminal style
        textView.font = NSFont.monospacedSystemFont(ofSize: config.fontSize, weight: .medium)
        textView.alignment = .left
        textView.string = config.content
        textView.textContainerInset = NSSize(width: 16, height: 16)

        // Add a colored border indicator
        let containerView = NSView(frame: NSRect(x: 0, y: 0, width: config.width, height: config.height))
        containerView.wantsLayer = true
        containerView.layer?.borderColor = NSColor(red: 1.0, green: 0.2, blue: 0.2, alpha: 0.8).cgColor
        containerView.layer?.borderWidth = 3.0
        containerView.layer?.cornerRadius = 4.0

        // Add status bar at bottom
        let statusLabel = NSTextField(labelWithString: "⬤ CAPTURE-INVISIBLE  |  sharingType = .none  |  Cmd+Q to quit")
        statusLabel.font = NSFont.monospacedSystemFont(ofSize: 11, weight: .regular)
        statusLabel.textColor = NSColor(red: 1.0, green: 0.4, blue: 0.4, alpha: 1.0)
        statusLabel.backgroundColor = NSColor(red: 0.05, green: 0.05, blue: 0.08, alpha: 1.0)
        statusLabel.isBezeled = false
        statusLabel.drawsBackground = true
        statusLabel.alignment = .center

        // Layout
        let scrollView = NSScrollView(frame: .zero)
        scrollView.documentView = textView
        scrollView.hasVerticalScroller = true
        scrollView.hasHorizontalScroller = false
        scrollView.autohidesScrollers = true
        scrollView.backgroundColor = NSColor(red: 0.1, green: 0.1, blue: 0.15, alpha: 1.0)

        containerView.addSubview(scrollView)
        containerView.addSubview(statusLabel)

        scrollView.translatesAutoresizingMaskIntoConstraints = false
        statusLabel.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            statusLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor, constant: 3),
            statusLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor, constant: -3),
            statusLabel.bottomAnchor.constraint(equalTo: containerView.bottomAnchor, constant: -3),
            statusLabel.heightAnchor.constraint(equalToConstant: 24),

            scrollView.topAnchor.constraint(equalTo: containerView.topAnchor, constant: 3),
            scrollView.leadingAnchor.constraint(equalTo: containerView.leadingAnchor, constant: 3),
            scrollView.trailingAnchor.constraint(equalTo: containerView.trailingAnchor, constant: -3),
            scrollView.bottomAnchor.constraint(equalTo: statusLabel.topAnchor, constant: -4),
        ])

        window.contentView = containerView
        window.makeKeyAndOrderFront(nil)

        self.window = window
    }

    func createStatusBarItem() {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem?.button?.title = "👁‍🗨"

        let menu = NSMenu()
        menu.addItem(NSMenuItem(title: "Invisible Window Active", action: nil, keyEquivalent: ""))
        menu.addItem(NSMenuItem.separator())

        let toggleItem = NSMenuItem(title: "Toggle sharingType", action: #selector(toggleSharing), keyEquivalent: "t")
        toggleItem.target = self
        menu.addItem(toggleItem)

        let captureTestItem = NSMenuItem(title: "Take Verification Screenshot", action: #selector(takeVerificationScreenshot), keyEquivalent: "s")
        captureTestItem.target = self
        menu.addItem(captureTestItem)

        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Quit", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q"))

        statusItem?.menu = menu
    }

    @objc func toggleSharing() {
        guard let window = self.window else { return }
        if window.sharingType == .none {
            window.sharingType = .readOnly
            print("[TOGGLE] sharingType → .readOnly (window NOW VISIBLE to capture)")
        } else {
            window.sharingType = .none
            print("[TOGGLE] sharingType → .none (window INVISIBLE to capture)")
        }
    }

    @objc func takeVerificationScreenshot() {
        let timestamp = Int(Date().timeIntervalSince1970)
        let path = "/tmp/invisible_window_verify_\(timestamp).png"

        let task = Process()
        task.launchPath = "/usr/sbin/screencapture"
        task.arguments = ["-x", path]  // -x = no sound
        task.launch()
        task.waitUntilExit()

        if task.terminationStatus == 0 {
            print("[VERIFY] Screenshot saved: \(path)")
            print("[VERIFY] If the invisible window does NOT appear in the screenshot,")
            print("[VERIFY] the display affinity bypass is working.")

            // Open the screenshot for visual comparison
            NSWorkspace.shared.open(URL(fileURLWithPath: path))
        } else {
            print("[ERROR] Screenshot failed with exit code \(task.terminationStatus)")
        }
    }

    func printStatus() {
        print("""

        ╔══════════════════════════════════════════════════════════╗
        ║         THE INVISIBLE WINDOW — macOS PoC Running        ║
        ╠══════════════════════════════════════════════════════════╣
        ║                                                          ║
        ║  Window created with sharingType = .none                 ║
        ║  Window level: floating (above other windows)            ║
        ║  Opacity: \(String(format: "%.0f%%", config.opacity * 100))                                            ║
        ║                                                          ║
        ║  macOS version: \(ProcessInfo.processInfo.operatingSystemVersionString)                    ║
        ║                                                          ║
        ║  Status bar menu (👁‍🗨):                                   ║
        ║    • Toggle sharingType (Cmd+T from menu)                ║
        ║    • Take verification screenshot (Cmd+S from menu)      ║
        ║    • Quit (Cmd+Q)                                        ║
        ║                                                          ║
        ║  TEST: Take a screenshot (Cmd+Shift+3) and check if     ║
        ║  this window appears. If sharingType = .none works,      ║
        ║  the window will be MISSING from the screenshot.         ║
        ║                                                          ║
        ╚══════════════════════════════════════════════════════════╝

        """)
    }
}

// MARK: - Main

let config = parseArgs()
let app = NSApplication.shared
app.setActivationPolicy(.accessory) // No dock icon — stealthier
let delegate = InvisibleWindowDelegate(config: config)
app.delegate = delegate
app.activate(ignoringOtherApps: true)
app.run()
