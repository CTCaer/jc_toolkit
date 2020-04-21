project(JCToolKit)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -D__jctool_cpp_API__
)

if(UNIX AND NOT APPLE) # Linux
    set(LINUX TRUE)
endif()

set(SOURCES
    jctool/jctool.cpp
    jctool/jctool_api_ui.cpp
    jctool/Controller.cpp
)

if (WIN32)
    set(SOURCES ${SOURCES}
        hidapi/windows/hid.c)
elseif(LINUX)
    set(SOURCES ${SOURCES}
        hidapi/linux/hid.c)
endif()

if (JCTOOL_API_ONLY)
    add_compile_definitions(__jctool_API_ONLY__)
    add_library(JCToolkit
        ${SOURCES}
    )
else()
    add_subdirectory(ui)
    add_executable(JCToolkit
        ${SOURCES}
    )
    target_link_libraries(JCToolkit
        ImGui
        Tools
    )
endif()

if(LINUX)
    target_link_libraries(JCToolkit
        udev
        pthread
    )
endif()


target_include_directories(JCToolkit PRIVATE
    jctool
    hidapi/hidapi
    ui/ImGui/imgui
    ui/ImGui/tools
    ui/ImGui/imgui/examples/libs/gl3w
    ui/ImGui/imgui/examples/libs/glfw/include
    ui/tools
    ui
    stb
)
