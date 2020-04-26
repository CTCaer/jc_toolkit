set(P imgui-tools)
project(${P})

add_library(${P} STATIC
    # Add the ImGui helpers
    imgui_helpers.cpp

    # Directory Explorer
    DirExplorer/dir_explorer.cpp
    DirExplorer/ImGuiDirExplorer.cpp

    # Image Load
    ImageLoad/ImageLoad.cpp
)

target_include_directories(${P} PRIVATE
    # Add the include directory for ImGui.
    ../imgui

    # Add the include directories for using OpenGL.
    ../imgui/examples/libs/gl3w
    ../imgui/examples/libs/glfw/include

    # Add the include directory for tools-squared.
    tools-squared
)
# Let's link the tools to ImGui.
target_link_libraries(${P}
    imgui-gl3w
)
