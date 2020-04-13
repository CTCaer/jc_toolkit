project(JCToolKit)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(
    __jctool_cpp_API__
    BATTERY_INDICATORS_PATH="${PROJECT_SOURCE_DIR}/jctool/original_res/batt_"
    BATTERY_INDICATORS_COUNT=10
    BATTERY_INDICATOR_NAMES="0 0_chr 25 25_chr 50 50_chr 75 75_chr 100 100_chr"
    BATTERY_INDICATORS_EXT=".png"
)

if (WIN32)
    set(HIDAPI_SOURCE hidapi/windows/hid.c)
endif()

if(CMAKE_SYSTEM_NAME EQUAL Linux)
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


target_include_directories(JCToolkit PRIVATE
    jctool
    hidapi/hidapi
    ui/ImGui/imgui
    ui/ImGui/ImGuiInterface
    ui/tools
)
