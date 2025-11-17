cmake_minimum_required(VERSION 3.10)
project(c2en VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    # MSVC compiler flags
    add_compile_options(/W4)
    # Remove /WX for warnings as errors to allow builds to succeed
else()
    # GCC/Clang compiler flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Remove -Werror to allow builds to succeed with warnings
endif()

# Release build optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3)
    endif()
endif()

# Source files
file(GLOB SOURCES "src/*.c")

# Main executable
add_executable(c2en ${SOURCES})

# Installation
install(TARGETS c2en DESTINATION bin)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "c2en")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C to British English Compiler")
set(CPACK_PACKAGE_VENDOR "Open Source")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

# Print build information
message(STATUS "")
message(STATUS "C to British English Compiler - Build Configuration")
message(STATUS "====================================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
