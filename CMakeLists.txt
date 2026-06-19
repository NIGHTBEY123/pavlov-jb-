cmake_minimum_required(VERSION 3.20)
project(CS2_Internal_Local LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
)

set(MINHOOK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/minhook)
set(MINHOOK_SOURCES
    ${MINHOOK_DIR}/hook.c
    ${MINHOOK_DIR}/buffer.c
    ${MINHOOK_DIR}/trampoline.c
    ${MINHOOK_DIR}/hde/hde64.c
)

add_library(${PROJECT_NAME} SHARED
    Source/DllMain.cpp
    ${IMGUI_SOURCES}
    ${MINHOOK_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${IMGUI_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/minhook/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE kernel32 user32 d3d11 dxgi winhttp)

target_compile_options(${PROJECT_NAME} PRIVATE /O2 /W0 /EHa /Ob0) # /Ob0 to disable some inlining that might make reversing easier to read structure
target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Obfuscation & Security settings for Release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
    # Remove debug symbols completely
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/DEBUG:NONE /PDBALTPATH:%_PDB% /OPT:REF /OPT:ICF"
    )
    # Optional: If you use MSVC, you can also strip symbols from the DLL
endif()
