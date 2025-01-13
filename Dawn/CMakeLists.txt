cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME "DrawInWindow")
project(${PROJECT_NAME})

set(SKIA_INCLUDE_PATH "D:\\sdk\\skia" CACHE PATH "Skia include path,default:D:/sdk/skia" FORCE)
set(SKIA_LIB_PATH "D:\\sdk\\skia\\out" CACHE PATH "Skia lib path,default:D:/sdk/skia/out" FORCE)

if(NOT IS_DIRECTORY "${SKIA_INCLUDE_PATH}")
    message(FATAL_ERROR "The path ${SKIA_INCLUDE_PATH} does not exist.")
endif()

if(NOT IS_DIRECTORY "${SKIA_LIB_PATH}")
    message(FATAL_ERROR "The path ${SKIA_LIB_PATH} does not exist.")
endif()



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

link_directories("${SKIA_LIB_PATH}")
include_directories("${SKIA_INCLUDE_PATH}" "D:\\sdk\\skia\\third_party\\externals\\dawn\\include" "D:\\sdk\\skia\\out\\debug\\gen\\third_party\\externals\\dawn\\include")
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE  
"main.cpp"
"DisplayParams.h"
"TestingImageProvider.h"
)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:UNICODE NDEBUG NOMINMAX _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS WIN32 _WINDOWS>)
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:UNICODE NDEBUG NOMINMAX _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS WIN32 _WINDOWS>)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
        skia 
        raw_ptr
        dawn_native_static 
        dawn_platform_static 
        dawn_proc_static
        spvtools_link
        spvtools_val 
        spvtools_reduce 
        spvtools_opt 
        spvtools 
        bentleyottmann
        utils
        common
        allocator_shim
        allocator_base
        allocator_core
        dxguid d3d11 d3d12 dxgi
        kernel32 user32 shell32 winmm)
elseif(APPLE)
endif()