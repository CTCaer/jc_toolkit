set(P JCToolKit)
project(${P})

set(CMAKE_CXX_STANDARD 17)

if(UNIX AND NOT APPLE) # Linux
    set(LINUX TRUE)
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

add_compile_options(
    -D__jctool_cpp_API__
)

set(SOURCES
    jctool/jctool.cpp
    jctool/jctool_ui.cpp
    jctool/IRSensor.cpp

    jctool/session/controller_session.cpp
)

if (JCTOOL_API_ONLY)
    add_compile_options(-D__jctool_API_ONLY__)
    add_library(${P}
        ${SOURCES}
    )
else()
    add_subdirectory(easy-imgui)
    add_executable(${P}
        ${SOURCES}
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
    easy-imgui/imgui
    easy-imgui/tools
    hidapi/hidapi
)
