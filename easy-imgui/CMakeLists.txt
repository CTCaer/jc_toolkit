set(P easy-imgui)
project(${P})

#################################
# Set up GL3W Loader for OpenGL #
#################################
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
add_library(GL3W_LOADER
    imgui/examples/libs/gl3w/GL/gl3w.c
)

if(WIN32)
    target_link_libraries(GL3W_LOADER
        ${OPENGL_gl_LIBRARY}
        ${PROJECT_SOURCE_DIR}/imgui/examples/libs/glfw/lib-vc2010-64/glfw3.lib
    )
elseif(LINUX)
    target_link_libraries(GL3W_LOADER
        ${OPENGL_gl_LIBRARY}
        ${CMAKE_DL_LIBS}
        glfw
    )
endif()

target_include_directories(GL3W_LOADER PRIVATE
    # For GL3W Loader
    imgui/examples/libs/gl3w
    imgui/examples/libs/glfw/include
)
#################################

#####################################
# Export ImGui as a static library. #
#####################################
add_compile_options(
    # Let ImGui know it will be used with GL3W.
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W
)
add_library(imgui-gl3w STATIC
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
)
target_include_directories(imgui-gl3w PRIVATE
    imgui
    imgui/examples/libs/gl3w
)
# Link ImGui with GL3W
target_link_libraries(imgui-gl3w
    GL3W_LOADER
)

# We are using some tools with ImGui.
add_subdirectory(tools)

# The bridge to ImGui.
add_library(${P}
    # Add the ImGui Interface.
    Interface/ImGuiMain.cpp
)
# Let easy-imgui know where to find header files provided by ImGui.
target_include_directories(${P} PRIVATE
    imgui
    # Get the required implementation to use GL3W+GLFW+OpenlGL
    # +ImGui with easy-imgui.
    imgui/examples
    imgui/examples/libs/gl3w
)

# Let's add the glue that forms a bond known as easy-imgui.
target_link_libraries(${P}
    imgui-gl3w
    imgui-tools
)