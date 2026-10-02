/*
 * Minimal zwlr_layer_shell_v1 protocol implementation.
 * Defines the wl_interface structs needed by the wayland client library.
 */
#include <wayland-client.h>
#include "wlr-layer-shell-protocol.h"

/* We reference these standard interfaces from wayland-client */
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_output_interface;

/* xdg_popup interface — only referenced by get_popup, which we don't use.
 * Declare it weak so we don't pull in xdg-shell just for this. */
static const struct wl_interface *no_interface = NULL;

/* ── Types arrays (one entry per argument in each message) ─────── */

/* get_layer_surface: new_id<layer_surface>, obj<surface>, ?obj<output>, uint, string */
static const struct wl_interface *ls_get_layer_surface_types[] = {
    &zwlr_layer_surface_v1_interface,
    &wl_surface_interface,
    &wl_output_interface,
    NULL,
    NULL,
};

/* get_popup: obj<xdg_popup> — we use NULL since we never call it */
static const struct wl_interface *ls_get_popup_types[] = {
    NULL, /* would be &xdg_popup_interface */
};

/* ── zwlr_layer_shell_v1 ─────────────────────────────────────────── */

static const struct wl_message zwlr_layer_shell_v1_requests[] = {
    { "get_layer_surface", "no?ous", ls_get_layer_surface_types },
    { "destroy",           "",       NULL },
};

const struct wl_interface zwlr_layer_shell_v1_interface = {
    "zwlr_layer_shell_v1", 4,
    2, zwlr_layer_shell_v1_requests,
    0, NULL,
};

/* ── zwlr_layer_surface_v1 ───────────────────────────────────────── */

static const struct wl_message zwlr_layer_surface_v1_requests[] = {
    { "set_size",                    "uu",   NULL },              /* 0 */
    { "set_anchor",                  "u",    NULL },              /* 1 */
    { "set_exclusive_zone",          "i",    NULL },              /* 2 */
    { "set_margin",                  "iiii", NULL },              /* 3 */
    { "set_keyboard_interactivity",  "u",    NULL },              /* 4 */
    { "get_popup",                   "o",    ls_get_popup_types },/* 5 */
    { "ack_configure",               "u",    NULL },              /* 6 */
    { "destroy",                     "",     NULL },              /* 7 */
    { "set_layer",                   "u",    NULL },              /* 8 */
};

static const struct wl_message zwlr_layer_surface_v1_events[] = {
    { "configure", "uuu", NULL },  /* 0: serial, width, height */
    { "closed",    "",    NULL },  /* 1 */
};

const struct wl_interface zwlr_layer_surface_v1_interface = {
    "zwlr_layer_surface_v1", 4,
    9, zwlr_layer_surface_v1_requests,
    2, zwlr_layer_surface_v1_events,
};