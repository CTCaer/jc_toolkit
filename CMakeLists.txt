project(JCToolKit)

add_subdirectory(ui)

add_compile_definitions(
    __jctool_cpp_API__
    BATTERY_INDICATORS_PATH="${PROJECT_SOURCE_DIR}/jctool/original_res/batt_"
    BATTERY_INDICATORS_COUNT=10
    BATTERY_INDICATOR_NAMES="0 0_chr 25 25_chr 50 50_chr 75 75_chr 100 100_chr"
    BATTERY_INDICATORS_EXT=".png"
)

set(SOURCES
    jctool/jctool.cpp
    jctool/hid.c
    
    jctool/jctool_api_ui.cpp
    jctool/Controller.cpp
)

add_executable(JCToolkit
    ${SOURCES}
)

target_link_libraries(JCToolkit
    ImGui
    Tools
)

target_include_directories(JCToolkit PRIVATE
    jctool
    ui/ImGui/imgui
    ui/ImGui/ImGuiInterface
    ui/tools
)
