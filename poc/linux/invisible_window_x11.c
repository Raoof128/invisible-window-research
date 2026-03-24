/*
 * invisible_window_x11.c
 * The Invisible Window — Linux/X11 Proof of Concept
 * Author: Mohammad Raouf Abedini <mohammadraouf.abedini@students.mq.edu.au>
 *
 * IMPORTANT: Linux does NOT have a direct equivalent to Windows
 * WDA_EXCLUDEFROMCAPTURE or macOS NSWindow.SharingType.none.
 *
 * This PoC explores TWO approaches on Linux:
 *
 * Approach A — X11 Override-Redirect Window:
 *   Creates an override-redirect window that bypasses the window manager.
 *   Some screen capture tools may not enumerate these properly.
 *   Result: PARTIAL — most capture tools still see it.
 *
 * Approach B — XComposite Redirection:
 *   Marks the window for manual compositing redirection. When a window is
 *   redirected with XCompositeRedirectWindow, the compositing manager is
 *   responsible for including it in the composite. Some capture APIs that
 *   read the root window pixmap may miss redirected windows.
 *   Result: VARIES by compositor and capture method.
 *
 * Approach C — Wayland (wp_viewporter / wl_surface):
 *   On Wayland, there is NO mechanism to hide a surface from capture.
 *   The Wayland protocol explicitly prevents applications from accessing
 *   other surfaces' content. However, screen capture goes through
 *   xdg-desktop-portal which captures the full compositor output.
 *   Result: NOT FEASIBLE on Wayland.
 *
 * CONCLUSION: Linux lacks OS-level display affinity APIs. The attack
 * described in the paper is primarily a Windows and macOS vulnerability.
 * This file documents the Linux landscape for completeness.
 *
 * Compile (X11): gcc invisible_window_x11.c -o invisible_window_x11 \
 *                    -lX11 -lXcomposite -lXfixes
 *
 * This is a research tool for authorized security testing only.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

/* Content lines */
static const char* CONTENT[] = {
    "+======================================================+",
    "|           THE INVISIBLE WINDOW - Linux PoC            |",
    "|                                                       |",
    "|  Linux does NOT have display affinity APIs equivalent  |",
    "|  to Windows WDA_EXCLUDEFROMCAPTURE or macOS            |",
    "|  NSWindow.SharingType.none.                           |",
    "|                                                       |",
    "|  This window tests two partial approaches:            |",
    "|  A) X11 Override-Redirect (bypasses WM)               |",
    "|  B) XComposite Redirection                            |",
    "|                                                       |",
    "|  Neither achieves full capture invisibility.           |",
    "|  The attack is NOT fully feasible on Linux.            |",
    "|                                                       |",
    "|  Press Q to quit | Press T to toggle override-redirect |",
    "+======================================================+",
};
static const int CONTENT_LINES = sizeof(CONTENT) / sizeof(CONTENT[0]);

/* Test approaches */
typedef enum {
    APPROACH_NORMAL,
    APPROACH_OVERRIDE_REDIRECT,
    APPROACH_COMPOSITE_REDIRECT
} Approach;

static Approach current_approach = APPROACH_OVERRIDE_REDIRECT;

void draw_window(Display* dpy, Window win, GC gc, int width, int height)
{
    /* Clear background — dark */
    XSetForeground(dpy, gc, 0x1A1A26);
    XFillRectangle(dpy, win, gc, 0, 0, width, height);

    /* Red border */
    XSetForeground(dpy, gc, 0xFF3333);
    XSetLineAttributes(dpy, gc, 3, LineSolid, CapRound, JoinRound);
    XDrawRectangle(dpy, win, gc, 2, 2, width - 4, height - 4);

    /* Green text */
    XSetForeground(dpy, gc, 0x00FF66);
    int y = 30;
    for (int i = 0; i < CONTENT_LINES; i++) {
        XDrawString(dpy, win, gc, 20, y, CONTENT[i], strlen(CONTENT[i]));
        y += 20;
    }

    /* Status bar */
    const char* status;
    switch (current_approach) {
        case APPROACH_OVERRIDE_REDIRECT:
            status = "[A] Override-Redirect: ACTIVE | T=toggle | Q=quit";
            break;
        case APPROACH_COMPOSITE_REDIRECT:
            status = "[B] XComposite Redirect: ACTIVE | T=toggle | Q=quit";
            break;
        default:
            status = "[N] Normal window (no evasion) | T=toggle | Q=quit";
    }
    XSetForeground(dpy, gc, 0x0D0D14);
    XFillRectangle(dpy, win, gc, 3, height - 28, width - 6, 25);
    XSetForeground(dpy, gc, 0xFF6666);
    XDrawString(dpy, win, gc, 10, height - 10, status, strlen(status));
}

int main(int argc, char** argv)
{
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "[ERROR] Cannot open X display. Are you running X11?\n");
        fprintf(stderr, "[INFO]  On Wayland, this PoC is not applicable.\n");
        fprintf(stderr, "[INFO]  Wayland has no display affinity mechanism.\n");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    int depth = DefaultDepth(dpy, screen);
    Visual* visual = DefaultVisual(dpy, screen);

    /* Check XComposite extension */
    int comp_event, comp_error;
    int has_composite = XCompositeQueryExtension(dpy, &comp_event, &comp_error);
    printf("[INFO] XComposite extension: %s\n", has_composite ? "available" : "NOT available");

    /* Center the window */
    int scr_w = DisplayWidth(dpy, screen);
    int scr_h = DisplayHeight(dpy, screen);
    int pos_x = (scr_w - WINDOW_WIDTH) / 2;
    int pos_y = (scr_h - WINDOW_HEIGHT) / 2;

    /* Create window with override-redirect (Approach A) */
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;  /* Bypass window manager */
    attrs.background_pixel = 0x1A1A26;
    attrs.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    Window win = XCreateWindow(
        dpy, root,
        pos_x, pos_y, WINDOW_WIDTH, WINDOW_HEIGHT,
        0,
        depth,
        InputOutput,
        visual,
        CWOverrideRedirect | CWBackPixel | CWEventMask,
        &attrs
    );

    /* Set window to stay on top */
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom wm_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(dpy, win, wm_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char*)&wm_above, 1);

    /* Approach B: XComposite redirection */
    if (has_composite) {
        XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
        printf("[INFO] XCompositeRedirectWindow applied (CompositeRedirectManual)\n");
    }

    /* Create GC */
    GC gc = XCreateGC(dpy, win, 0, NULL);

    /* Map window */
    XMapRaised(dpy, win);
    XFlush(dpy);

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         THE INVISIBLE WINDOW — Linux/X11 PoC Running   ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║                                                        ║\n");
    printf("║  Approach A: Override-Redirect = %s                 ║\n",
           attrs.override_redirect ? "ON " : "OFF");
    printf("║  Approach B: XComposite = %s                       ║\n",
           has_composite ? "ON " : "N/A");
    printf("║                                                        ║\n");
    printf("║  NOTE: Linux lacks true display affinity APIs.         ║\n");
    printf("║  Neither approach achieves full capture invisibility.   ║\n");
    printf("║                                                        ║\n");
    printf("║  Press T to cycle approaches | Q to quit               ║\n");
    printf("║                                                        ║\n");
    printf("║  TEST: Take a screenshot (gnome-screenshot,            ║\n");
    printf("║  scrot, flameshot) and check if this window appears.   ║\n");
    printf("║                                                        ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Event loop */
    int running = 1;
    while (running) {
        XEvent event;
        XNextEvent(dpy, &event);

        switch (event.type) {
        case Expose:
            draw_window(dpy, win, gc, WINDOW_WIDTH, WINDOW_HEIGHT);
            break;

        case KeyPress: {
            KeySym key = XLookupKeysym(&event.xkey, 0);
            if (key == XK_q || key == XK_Q || key == XK_Escape) {
                running = 0;
            } else if (key == XK_t || key == XK_T) {
                /* Cycle approaches */
                current_approach = (current_approach + 1) % 3;
                const char* names[] = {"NORMAL", "OVERRIDE_REDIRECT", "COMPOSITE_REDIRECT"};
                printf("[TOGGLE] Approach: %s\n", names[current_approach]);

                /* Recreate window with new attributes */
                attrs.override_redirect = (current_approach == APPROACH_OVERRIDE_REDIRECT);
                XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &attrs);

                if (has_composite) {
                    if (current_approach == APPROACH_COMPOSITE_REDIRECT) {
                        XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
                        printf("[INFO] XComposite redirect applied\n");
                    } else {
                        XCompositeUnredirectWindow(dpy, win, CompositeRedirectManual);
                        printf("[INFO] XComposite redirect removed\n");
                    }
                }

                /* Remap to apply changes */
                XUnmapWindow(dpy, win);
                XMapRaised(dpy, win);
                XFlush(dpy);
            }
            break;
        }

        default:
            break;
        }
    }

    /* Cleanup */
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    printf("[EXIT] Linux PoC terminated.\n");
    return 0;
}
