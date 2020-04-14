project(JCToolKit)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -D__jctool_cpp_API__
    -DBATTERY_INDICATORS_PATH="${PROJECT_SOURCE_DIR}/jctool/original_res/batt_"
    -DBATTERY_INDICATORS_COUNT=10
    -DBATTERY_INDICATOR_NAMES="0 0_chr 25 25_chr 50 50_chr 75 75_chr 100 100_chr"
    -DBATTERY_INDICATORS_EXT=".png"
)

if(UNIX AND NOT APPLE) # Linux
    set(LINUX TRUE)
endif()

if (WIN32)
    set(HIDAPI_SOURCE hidapi/windows/hid.c)
elseif(LINUX)
    set(HIDAPI_SOURCE hidapi/linux/hid.c)
endif()

set(SOURCES
    jctool/jctool.cpp
    ${HIDAPI_SOURCE}

    jctool/jctool_api_ui.cpp
    jctool/Controller.cpp
)

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
    )
endif()


target_include_directories(JCToolkit PRIVATE
    jctool
    hidapi/hidapi
    ui/ImGui/imgui
    ui/ImGui/ImGuiInterface
    ui/tools
)
