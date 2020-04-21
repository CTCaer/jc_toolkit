project(ImGui)

##############################
# Export ImGui as a library. #
##############################
add_compile_options(
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W
    -DIMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/tools/this_is_imconfig.h"
)
add_library(ImGui STATIC
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

    # Add the ImGui helpers
    tools/imgui_helpers.cpp

    # Add the ImGui Interface.
    tools/Interface/ImGuiMain.cpp

    # Directory Explorer
    tools/DirExplorer/ImGuiDirExplorer.cpp
)
target_link_libraries(ImGui
    GL3W_LOADER
)

target_include_directories(ImGui PRIVATE
    imgui
    imgui/examples
    imgui/examples/libs/gl3w
    imgui/examples/libs/glfw/include
    ../tools
)
