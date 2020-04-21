#pragma once
/**
 * 
 */

enum ImGuiInitFlags {
    NoWindowDecoration = 1,
    ShowDemoWindow = 2
};

using ImGuiCallsCB = int (*)();
struct WindowInit {
    const char* title;
    int width;
    int height;
};
/**
 * When imgui_calls() returns a value < 0, then shutdown ImGui.
 */
int ImGuiMain(WindowInit window_init, ImGuiCallsCB imgui_calls, void (*on_graphics_init)(), ImGuiInitFlags init_flags);