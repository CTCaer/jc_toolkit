set(P JCToolKit)
project(${P})

set(CMAKE_CXX_STANDARD 17)

if(UNIX AND NOT APPLE) # Linux
    set(LINUX TRUE)
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL$<$<CONFIG:Debug>:Debug>")
endif()

###########################
# We will be using hidapi #
###########################
# Find the proper hidapi source file for use on the target platform.
if (WIN32)
    set(HIDAPI_SOURCE ${SOURCES}
        hidapi/windows/hid.c)
elseif(LINUX)
    set(HIDAPI_SOURCE ${SOURCES}
        hidapi/linux/hid.c)
endif()

add_library(hidapi STATIC
    ${HIDAPI_SOURCE}
)
target_include_directories(hidapi PRIVATE
    hidapi/hidapi
)

if(WIN32)
    target_link_libraries(hidapi
        setupapi
    )
endif()

add_compile_options(
    -D__jctool_disable_legacy_ui__
)

set(SOURCES
    jctool/jctool.cpp
    jctool/jctool_input.cpp
    jctool/jctool_irsensor.cpp
    jctool/jctool_leds.cpp
    jctool/jctool_mcu.cpp
    jctool/jctool_nfc.cpp
    jctool/jctool_rumble.cpp
    jctool/IRSensor.cpp
    
    jctool/session/controller_session.cpp
    jctool/com-builder/com_builder.cpp
)

if (JCTOOL_NO_UI)
    add_compile_options(-D__jctool_no_UI__)
    add_library(${P}
        ${SOURCES}
    )
else()
    add_subdirectory(easy-imgui)
    add_executable(${P}
        ${SOURCES}
        jctool/com-builder/com_builder_ui.cpp
        jctool/jctool_ui.cpp
    )
    target_link_libraries(${P}
        easy-imgui
    )
endif()
# Link JCToolkit with hidapi.
target_link_libraries(${P}
    hidapi
)

if(LINUX)
    target_link_libraries(${P}
        udev
        pthread
    )
endif()


target_include_directories(${P} PRIVATE
    ./
    jctool
    jctool/include
    easy-imgui/imgui
    easy-imgui/tools
    hidapi/hidapi
)
