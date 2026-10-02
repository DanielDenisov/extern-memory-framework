#pragma once

/*
 * window.h — Wayland-native Window class with universal overlay support.
 *
 * Overlay mode uses zwlr_layer_shell_v1 (the wlr-layer-shell protocol) to
 * place the surface on the compositor's overlay layer.  This works without
 * compositor-specific window rules on:
 *   - KDE Plasma 5.27+ (Wayland)
 *   - Hyprland
 *   - Sway
 *   - All wlroots-based compositors
 *
 * Windowed mode uses GLFW normally.
 *
 * Build requirements (overlay):
 *   pkg-config --cflags --libs wayland-client wayland-egl egl
 *   Compile protocols/wlr-layer-shell-protocol.c and link it in.
 *
 * Build requirements (windowed):
 *   pkg-config --cflags --libs glfw3
 */

#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdlib.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <cstring>
#include <chrono>

#include "config.h"
#include "protocols/wlr-layer-shell-protocol.h"

class Window {
private:
    /* Shared */
    bool overlay{false};
    int width{settings::WINDOW_WIDTH};
    int height{settings::WINDOW_WIDTH};

    /* GLFW path (windowed mode) */
    GLFWwindow* glfw_window = nullptr;

    /* Layer-shell path (overlay mode) */
    struct wl_display*              wl_dpy          = nullptr;
    struct wl_registry*             wl_reg          = nullptr;
    struct wl_compositor*           wl_comp         = nullptr;
    struct zwlr_layer_shell_v1*     layer_shell     = nullptr;
    struct wl_surface*              wl_surf         = nullptr;
    struct zwlr_layer_surface_v1*   layer_surface   = nullptr;
    struct wl_egl_window*           egl_win         = nullptr;
    EGLDisplay                      egl_dpy         = EGL_NO_DISPLAY;
    EGLContext                      egl_ctx         = EGL_NO_CONTEXT;
    EGLSurface                      egl_surf        = EGL_NO_SURFACE;
    bool                            configured      = false;
    bool                            closed          = false;

    using clock = std::chrono::steady_clock;
    clock::time_point last_frame = clock::now();

    /* ── Wayland registry listener ──────────────────────────────── */
    static void registry_global(void* data, struct wl_registry* reg,
                                uint32_t name, const char* iface, uint32_t ver) {
        auto* self = static_cast<Window*>(data);
        if (strcmp(iface, "wl_compositor") == 0) {
            self->wl_comp = static_cast<struct wl_compositor*>(
                wl_registry_bind(reg, name, &wl_compositor_interface, 4));
        } else if (strcmp(iface, "zwlr_layer_shell_v1") == 0) {
            self->layer_shell = static_cast<struct zwlr_layer_shell_v1*>(
                wl_registry_bind(reg, name, &zwlr_layer_shell_v1_interface,
                                 ver < 4 ? ver : 4));
        }
    }
    static void registry_global_remove(void*, struct wl_registry*, uint32_t) {}

    static constexpr struct wl_registry_listener reg_listener = {
        registry_global, registry_global_remove
    };

    /* ── Layer surface listener ─────────────────────────────────── */
    static void layer_configure(void* data, struct zwlr_layer_surface_v1* surf,
                                uint32_t serial, uint32_t w, uint32_t h) {
        auto* self = static_cast<Window*>(data);
        if (w > 0) self->width  = static_cast<int>(w);
        if (h > 0) self->height = static_cast<int>(h);

        zwlr_layer_surface_v1_ack_configure(surf, serial);

        if (self->egl_win) {
            wl_egl_window_resize(self->egl_win, self->width, self->height, 0, 0);
        }
        self->configured = true;
    }

    static void layer_closed(void* data, struct zwlr_layer_surface_v1*) {
        static_cast<Window*>(data)->closed = true;
    }

    static constexpr struct zwlr_layer_surface_v1_listener layer_listener = {
        layer_configure, layer_closed
    };

    /* ── EGL helpers ────────────────────────────────────────────── */
    bool initEGL() {
        egl_dpy = eglGetDisplay(static_cast<EGLNativeDisplayType>(wl_dpy));
        if (egl_dpy == EGL_NO_DISPLAY) {
            std::cerr << "[-] eglGetDisplay failed" << std::endl;
            return false;
        }

        EGLint major, minor;
        if (!eglInitialize(egl_dpy, &major, &minor)) {
            std::cerr << "[-] eglInitialize failed" << std::endl;
            return false;
        }

        eglBindAPI(EGL_OPENGL_API);

        // Request a config with alpha for transparency
        EGLint config_attribs[] = {
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,      8,
            EGL_BLUE_SIZE,       8,
            EGL_ALPHA_SIZE,      8,
            EGL_NONE,
        };

        EGLConfig egl_config;
        EGLint num_configs;
        if (!eglChooseConfig(egl_dpy, config_attribs, &egl_config, 1, &num_configs)
            || num_configs == 0) {
            std::cerr << "[-] eglChooseConfig failed" << std::endl;
            return false;
        }

        // OpenGL 3.0 context (matches GLFW path)
        EGLint ctx_attribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 0,
            EGL_NONE,
        };

        egl_ctx = eglCreateContext(egl_dpy, egl_config, EGL_NO_CONTEXT, ctx_attribs);
        if (egl_ctx == EGL_NO_CONTEXT) {
            std::cerr << "[-] eglCreateContext failed" << std::endl;
            return false;
        }

        // Create the EGL window surface from our wl_egl_window
        egl_win = wl_egl_window_create(wl_surf, width, height);
        if (!egl_win) {
            std::cerr << "[-] wl_egl_window_create failed" << std::endl;
            return false;
        }

        egl_surf = eglCreateWindowSurface(egl_dpy, egl_config,
                                           (EGLNativeWindowType)egl_win, NULL);
        if (egl_surf == EGL_NO_SURFACE) {
            std::cerr << "[-] eglCreateWindowSurface failed" << std::endl;
            return false;
        }

        eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx);
        // No vsync — same rationale as GLFW path
        eglSwapInterval(egl_dpy, 0);

        return true;
    }

    /* ── Layer-shell overlay init ───────────────────────────────── */
    bool initOverlay() {
        width  = settings::WINDOW_WIDTH;
        height = settings::WINDOW_HEIGHT;

        // Connect to the Wayland display
        wl_dpy = wl_display_connect(NULL);
        if (!wl_dpy) {
            std::cerr << "[-] Failed to connect to Wayland display" << std::endl;
            return false;
        }

        // Bind globals (wl_compositor, zwlr_layer_shell_v1)
        wl_reg = wl_display_get_registry(wl_dpy);
        wl_registry_add_listener(wl_reg, &reg_listener, this);
        wl_display_roundtrip(wl_dpy);

        if (!wl_comp) {
            std::cerr << "[-] Wayland compositor interface not found" << std::endl;
            return false;
        }

        if (!layer_shell) {
            std::cerr << "[-] zwlr_layer_shell_v1 not supported by compositor." << std::endl;
            std::cerr << "    Supported: KDE Plasma 5.27+, Hyprland, Sway, wlroots-based." << std::endl;
            std::cerr << "    Falling back to GLFW overlay (may need compositor rules)." << std::endl;
            // Clean up wayland objects before falling back
            if (wl_reg) wl_registry_destroy(wl_reg);
            wl_display_disconnect(wl_dpy);
            wl_dpy = nullptr;
            wl_reg = nullptr;
            wl_comp = nullptr;
            return false;
        }

        // Create surface
        wl_surf = wl_compositor_create_surface(wl_comp);
        if (!wl_surf) {
            std::cerr << "[-] Failed to create wl_surface" << std::endl;
            return false;
        }

        // Create layer surface on the OVERLAY layer
        layer_surface = zwlr_layer_shell_v1_get_layer_surface(
            layer_shell,
            wl_surf,
            NULL,  // NULL = all outputs (primary)
            ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
            settings::name
        );

        if (!layer_surface) {
            std::cerr << "[-] Failed to create layer surface" << std::endl;
            return false;
        }

        // Configure: anchor to all edges = fullscreen, exclusive zone -1 = don't reserve space
        zwlr_layer_surface_v1_set_size(layer_surface, width, height);
        zwlr_layer_surface_v1_set_anchor(layer_surface,
            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP  | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
        zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
        zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface,
            ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

        zwlr_layer_surface_v1_add_listener(layer_surface, &layer_listener, this);

        // Commit to trigger the configure event
        wl_surface_commit(wl_surf);
        wl_display_roundtrip(wl_dpy);

        if (!configured) {
            std::cerr << "[-] Layer surface configure not received" << std::endl;
            return false;
        }

        // Init EGL on our surface
        if (!initEGL()) return false;

        // Init ImGui (OpenGL backend only — no GLFW backend for overlay)
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);

        // We use the OpenGL3 backend; input is handled manually (none needed for overlay)
        ImGui_ImplOpenGL3_Init("#version 130");

        // Set display size
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

        std::cout << "[+] Overlay Initialized (Wayland Layer-Shell, "
                  << width << "x" << height << ")" << std::endl;
        return true;
    }

    /* ── GLFW windowed init ─────────────────────────────────────── */
    bool initWindowed() {
        setenv("GLFW_PLATFORM", "wayland", 1);

        if (!glfwInit()) {
            std::cerr << "[-] GLFW Init Failed!" << std::endl;
            return false;
        }

#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
        if (glfwGetPlatform() != GLFW_PLATFORM_WAYLAND) {
            std::cerr << "[-] FATAL: GLFW in X11 mode!" << std::endl;
            glfwTerminate();
            return false;
        }
#endif

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

        glfw_window = glfwCreateWindow(800, 800, "Application", nullptr, nullptr);
        if (!glfw_window) {
            std::cerr << "[-] Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(glfw_window);
        glfwSwapInterval(0);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);

        ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        std::cout << "[+] Window Initialized (Wayland GLFW)" << std::endl;
        return true;
    }

    /* ── GLFW fallback overlay (needs compositor rules) ─────────── */
    bool initFallbackOverlay() {
        setenv("GLFW_PLATFORM", "wayland", 1);

        if (!glfwInit()) {
            std::cerr << "[-] GLFW Init Failed!" << std::endl;
            return false;
        }

#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
        if (glfwGetPlatform() != GLFW_PLATFORM_WAYLAND) {
            std::cerr << "[-] FATAL: GLFW in X11 mode!" << std::endl;
            glfwTerminate();
            return false;
        }
#endif

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

        width  = settings::WINDOW_WIDTH;
        height = settings::WINDOW_HEIGHT;

        glfw_window = glfwCreateWindow(width, height, settings::name, nullptr, nullptr);
        if (!glfw_window) {
            std::cerr << "[-] Failed to create GLFW overlay window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwSetWindowPos(glfw_window, 0, 0);
        glfwMakeContextCurrent(glfw_window);
        glfwSetWindowAttrib(glfw_window, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        glfwSwapInterval(0);
        glfwShowWindow(glfw_window);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);

        ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        std::cerr << "[!] Overlay using GLFW fallback — may need compositor window rules" << std::endl;
        std::cout << "[+] Overlay Initialized (GLFW Fallback)" << std::endl;
        return true;
    }

public:
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    ~Window() {
        ImGui_ImplOpenGL3_Shutdown();
        if (glfw_window) {
            ImGui_ImplGlfw_Shutdown();
        }
        ImGui::DestroyContext();

        /* Layer-shell cleanup */
        if (egl_surf != EGL_NO_SURFACE) eglDestroySurface(egl_dpy, egl_surf);
        if (egl_ctx  != EGL_NO_CONTEXT) eglDestroyContext(egl_dpy, egl_ctx);
        if (egl_dpy  != EGL_NO_DISPLAY) eglTerminate(egl_dpy);
        if (egl_win)       wl_egl_window_destroy(egl_win);
        if (layer_surface) zwlr_layer_surface_v1_destroy(layer_surface);
        if (wl_surf)       wl_surface_destroy(wl_surf);
        if (layer_shell)   zwlr_layer_shell_v1_destroy(layer_shell);
        if (wl_comp)       wl_compositor_destroy(wl_comp);
        if (wl_reg)        wl_registry_destroy(wl_reg);
        if (wl_dpy)        wl_display_disconnect(wl_dpy);

        /* GLFW cleanup */
        if (glfw_window) {
            glfwDestroyWindow(glfw_window);
            glfwTerminate();
        }
    }

    Window(bool overlay = false) : overlay(overlay) {
        if (overlay) {
            // Try layer-shell first (universal), fall back to GLFW hints
            if (!initOverlay()) {
                if (!initFallbackOverlay()) {
                    std::cerr << "[-] All overlay init methods failed" << std::endl;
                }
            }
        } else {
            if (!initWindowed()) {
                std::cerr << "[-] Windowed init failed" << std::endl;
            }
        }
    }

    bool IsValid() const {
        return glfw_window != nullptr || (wl_dpy != nullptr && configured);
    }

    bool ShouldClose() const {
        if (glfw_window) return glfwWindowShouldClose(glfw_window);
        return closed;
    }

    void RenderBegin() {
        if (glfw_window) {
            /* GLFW path */
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        } else {
            /* Layer-shell path — dispatch wayland events ourselves */
            wl_display_dispatch_pending(wl_dpy);
            wl_display_flush(wl_dpy);

            /* Update ImGui timing manually (no GLFW backend) */
            auto now = clock::now();
            float dt = std::chrono::duration<float>(now - last_frame).count();
            last_frame = now;

            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(static_cast<float>(width),
                                     static_cast<float>(height));
            io.DeltaTime = dt > 0.0f ? dt : 1.0f / 60.0f;

            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
        }

        if (this->overlay) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(
                static_cast<float>(width), static_cast<float>(height)));
            ImGui::Begin("##Overlay", nullptr,
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus
            );
        } else {
            ImGui::Begin("Application Control Panel", nullptr);
        }
    }

    void RenderEnd() {
        ImGui::End();
        ImGui::Render();

        if (glfw_window) {
            /* GLFW path */
            int w, h;
            glfwGetFramebufferSize(glfw_window, &w, &h);
            glViewport(0, 0, w, h);

            if (this->overlay) {
                glClearColor(0, 0, 0, 0);
            } else {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            }

            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(glfw_window);
        } else {
            /* Layer-shell path */
            glViewport(0, 0, width, height);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            eglSwapBuffers(egl_dpy, egl_surf);
        }
    }
};


/* ── Free drawing helpers ─────────────────────────────────────────── */

inline void DrawCircleFilled(float x, float y, float radius, ImU32 color) {
    ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(x, y), radius, color, 0);
}

inline void DrawBox(float x, float y, float w, float h, ImU32 color, float thickness = 1.5f) {
    ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
}

inline void DrawLine(float x1, float y1, float x2, float y2, ImU32 color, float thickness = 1.5f) {
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
}

inline void DrawTextImGui(float x, float y, ImU32 color, const char* text) {
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), color, text);
}

inline void DrawTextCentered(float x, float y, ImU32 color, const char* text) {
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 pos = ImVec2(x - (textSize.x / 2.0f), y - (textSize.y / 2.0f));
    ImGui::GetBackgroundDrawList()->AddText(pos, color, text);
}