project(ui)

find_package(OpenGL REQUIRED)
add_library(GL3W_LOADER
    ImGui/imgui/examples/libs/gl3w/GL/gl3w.c
)
target_link_libraries(GL3W_LOADER
    ${OPENGL_gl_LIBRARY}
    ${PROJECT_SOURCE_DIR}/ImGui/imgui/examples/libs/glfw/lib-vc2010-64/glfw3.lib
)
target_include_directories(GL3W_LOADER PRIVATE
    ImGui/imgui/examples/libs/gl3w
    ImGui/imgui/examples/libs/glfw/include
)

# ImGui (as a library.)
add_subdirectory(ImGui)

# Load images asynchronously on a seperate thread.
#add_compile_definitions(ImageLoadASYNC)

# Miscellaneous tools
add_library(Tools
    tools/ImageLoad/ImageLoad.cpp
)
target_include_directories(Tools PRIVATE
    tools
    ImGui/imgui/examples/libs/gl3w
    ImGui/imgui/examples/libs/glfw/include
)
