cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME "EncryptSaveAndDecryptDemo")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME})
add_subdirectory(src)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:UNICODE _UNICODE WIN32 _WINDOWS DEBUG>)
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:UNICODE _UNICODE WIN32 _WINDOWS NDEBUG >)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} Crypt32 Advapi32)
elseif(APPLE)
endif()