/*
 * Minimal zwlr_layer_shell_v1 protocol bindings.
 * Equivalent to wayland-scanner output from wlr-layer-shell-unstable-v1.xml
 *
 * If you have wlr-protocols installed, you can generate this instead:
 *   wayland-scanner client-header < /path/to/wlr-layer-shell-unstable-v1.xml > wlr-layer-shell-protocol.h
 *   wayland-scanner private-code  < /path/to/wlr-layer-shell-unstable-v1.xml > wlr-layer-shell-protocol.c
 */
#pragma once

#include <wayland-client.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Forward declarations ─────────────────────────────────────────── */
struct zwlr_layer_shell_v1;
struct zwlr_layer_surface_v1;

/* ── Enums ────────────────────────────────────────────────────────── */

enum zwlr_layer_shell_v1_layer {
    ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND = 0,
    ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM     = 1,
    ZWLR_LAYER_SHELL_V1_LAYER_TOP        = 2,
    ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY    = 3,
};

enum zwlr_layer_surface_v1_anchor {
    ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP    = 1,
    ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM = 2,
    ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT   = 4,
    ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT  = 8,
};

enum zwlr_layer_surface_v1_keyboard_interactivity {
    ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE      = 0,
    ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE = 1,
    ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND = 2,
};

/* ── Interface declarations (defined in .c) ───────────────────────── */

extern const struct wl_interface zwlr_layer_shell_v1_interface;
extern const struct wl_interface zwlr_layer_surface_v1_interface;

/* ── zwlr_layer_surface_v1 listener ───────────────────────────────── */

struct zwlr_layer_surface_v1_listener {
    /**
     * configure - compositor tells us the size; we must ack
     */
    void (*configure)(void *data,
                      struct zwlr_layer_surface_v1 *surface,
                      uint32_t serial,
                      uint32_t width,
                      uint32_t height);
    /**
     * closed - compositor wants us to close
     */
    void (*closed)(void *data,
                   struct zwlr_layer_surface_v1 *surface);
};

static inline int
zwlr_layer_surface_v1_add_listener(struct zwlr_layer_surface_v1 *surface,
                                   const struct zwlr_layer_surface_v1_listener *listener,
                                   void *data) {
    return wl_proxy_add_listener((struct wl_proxy *)surface,
                                 (void (**)(void))listener, data);
}

/* ── zwlr_layer_shell_v1 requests ─────────────────────────────────── */

/* opcode 0: get_layer_surface */
static inline struct zwlr_layer_surface_v1 *
zwlr_layer_shell_v1_get_layer_surface(struct zwlr_layer_shell_v1 *shell,
                                       struct wl_surface *surface,
                                       struct wl_output *output,
                                       uint32_t layer,
                                       const char *name_space) {
    struct wl_proxy *p;
    p = wl_proxy_marshal_constructor(
        (struct wl_proxy *)shell,
        0, /* opcode */
        &zwlr_layer_surface_v1_interface,
        NULL, surface, output, layer, name_space);
    return (struct zwlr_layer_surface_v1 *)p;
}

/* opcode 1: destroy */
static inline void
zwlr_layer_shell_v1_destroy(struct zwlr_layer_shell_v1 *shell) {
    wl_proxy_marshal((struct wl_proxy *)shell, 1);
    wl_proxy_destroy((struct wl_proxy *)shell);
}

/* ── zwlr_layer_surface_v1 requests ───────────────────────────────── */

/* opcode 0: set_size */
static inline void
zwlr_layer_surface_v1_set_size(struct zwlr_layer_surface_v1 *surface,
                                uint32_t width, uint32_t height) {
    wl_proxy_marshal((struct wl_proxy *)surface, 0, width, height);
}

/* opcode 1: set_anchor */
static inline void
zwlr_layer_surface_v1_set_anchor(struct zwlr_layer_surface_v1 *surface,
                                  uint32_t anchor) {
    wl_proxy_marshal((struct wl_proxy *)surface, 1, anchor);
}

/* opcode 2: set_exclusive_zone */
static inline void
zwlr_layer_surface_v1_set_exclusive_zone(struct zwlr_layer_surface_v1 *surface,
                                          int32_t zone) {
    wl_proxy_marshal((struct wl_proxy *)surface, 2, zone);
}

/* opcode 3: set_margin */
static inline void
zwlr_layer_surface_v1_set_margin(struct zwlr_layer_surface_v1 *surface,
                                  int32_t top, int32_t right,
                                  int32_t bottom, int32_t left) {
    wl_proxy_marshal((struct wl_proxy *)surface, 3, top, right, bottom, left);
}

/* opcode 4: set_keyboard_interactivity */
static inline void
zwlr_layer_surface_v1_set_keyboard_interactivity(
    struct zwlr_layer_surface_v1 *surface, uint32_t mode) {
    wl_proxy_marshal((struct wl_proxy *)surface, 4, mode);
}

/* opcode 5: get_popup (we skip this — not needed for overlay) */

/* opcode 6: ack_configure */
static inline void
zwlr_layer_surface_v1_ack_configure(struct zwlr_layer_surface_v1 *surface,
                                     uint32_t serial) {
    wl_proxy_marshal((struct wl_proxy *)surface, 6, serial);
}

/* opcode 7: destroy */
static inline void
zwlr_layer_surface_v1_destroy(struct zwlr_layer_surface_v1 *surface) {
    wl_proxy_marshal((struct wl_proxy *)surface, 7);
    wl_proxy_destroy((struct wl_proxy *)surface);
}

#ifdef __cplusplus
}
#endif