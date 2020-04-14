project(ImGui)

##############################
# Export ImGui as a library. #
##############################
add_compile_options(
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W
)
add_library(ImGui
    # Add ImGui source files.
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp

    # Use OpenGL3 as the graphics renderer along with GLFWimpl as a platform binding.
    imgui/examples/imgui_impl_opengl3.cpp
    imgui/examples/imgui_impl_glfw.cpp

    # Allow the use of std::string alongside C-style strings.
    imgui/misc/cpp/imgui_stdlib.cpp

    # Add the ImGui Interface.
    ImGuiInterface/ImGuiMain.cpp
)
target_link_libraries(ImGui
    GL3W_LOADER
)

target_include_directories(ImGui PRIVATE
    imgui
    imgui/examples
    imgui/examples/libs/gl3w
    imgui/examples/libs/glfw/include
)
