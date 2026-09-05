#pragma once

#define GLFW_EXPOSE_NATIVE_WAYLAND //Must be defined before imports of GLFW
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdlib.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include "config.h"

class Window {
private:
    GLFWwindow* window = nullptr;
    bool overlay{false};
public:
    ~Window() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    Window(bool overlay = false) {
        overlay = false;
        //Force wayland
        setenv("GLFW_PLATFORM", "wayland", 1);

        if (!glfwInit()) {
            std::cerr << "[-] GLFW Init Failed! Install 'libglfw3-dev' and 'libwayland-dev'." << std::endl;
            window = nullptr; //fail condition
        }

        // Verify wayland
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
        if (glfwGetPlatform() != GLFW_PLATFORM_WAYLAND) {
            std::cerr << "[-] FATAL: GLFW initialized into X11 mode! Aborting." << std::endl;
            glfwTerminate();
            window = nullptr;
        }
#endif

        if (this->overlay) {
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

            window = glfwCreateWindow(settings::WINDOW_WIDTH, settings::WINDOW_HEIGHT, settings::name.c_str(), nullptr, nullptr);
        } else {
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
            glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
            glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
            glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
            glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

            window = glfwCreateWindow(800, 800, settings::name.c_str(), nullptr, nullptr);
        }

        window = glfwCreateWindow(settings::WINDOW_WIDTH, settings::WINDOW_HEIGHT, settings::name.c_str(), nullptr, nullptr);
        if (!window) return; //exit state

        glfwSetWindowPos(window, 0, 0);

        glfwMakeContextCurrent(window);
        if (overlay) {
            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        } else {
            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
        }


        glfwSwapInterval(0);
        glfwShowWindow(window);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0f;
        if (this->overlay) {
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
        } else {
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
        }

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        std::cout << "[+] Window Initialized (Wayland Native)" << std::endl;
    }

    bool ShouldClose() {
        return glfwWindowShouldClose(window);
    }

    void RenderBegin() {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        if (this->overlay) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(settings::WINDOW_WIDTH, settings::WINDOW_HEIGHT));

            ImGui::Begin("##Overlay", nullptr,
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus
            );
        } else {
            // ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            // ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            ImGui::Begin("Application Control Panel", nullptr);
        }
    }

    void RenderEnd() {
        ImGui::End();
        ImGui::Render();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        if (this->overlay) {
            glClearColor(0, 0, 0, 0);
        } else {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
};


void DrawCircleFilled(float x, float y, float radius, ImU32 color) {
    ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(x, y), radius, color, 0);
}

void DrawBox(float x, float y, float w, float h, ImU32 color, float thickness = 1.5f) {
    ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
}

void DrawLine(float x1, float y1, float x2, float y2, ImU32 color, float thickness = 1.5f) {
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
}

void DrawTextImGui(float x, float y, ImU32 color, const char* text) {
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), color, text);
}

void DrawTextCentered(float x, float y, ImU32 color, const char* text) {
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 pos = ImVec2(x - (textSize.x / 2.0f), y - (textSize.y / 2.0f));
    ImGui::GetBackgroundDrawList()->AddText(pos, color, text);
}