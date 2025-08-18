# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("descansa")

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIXED: Add 16KB page size alignment for Android compatibility
# This ensures the shared library works on devices with 16KB page sizes
if(ANDROID)
    # Set linker flags for 16KB alignment
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,common-page-size=16384")

    # Additional alignment flags for all architectures
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,max-page-size=16384")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,common-page-size=16384")
endif()

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp
        DescansaCore.cpp
        SleepDataStructures.cpp
        DescansaCoreManager.cpp
        SleepAnalyticsEngine.cpp
        ThemeManager.cpp)

# Include directories for headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log)

# FIXED: Enhanced compiler flags for better compatibility and 16KB alignment
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-unused-function
        -std=c++11
        # Add optimization flags that respect alignment
        $<$<CONFIG:Release>:-O2 -ffunction-sections -fdata-sections>
        $<$<CONFIG:Debug>:-g -O0>)

# FIXED: Target-specific linker flags for 16KB alignment
if(ANDROID)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
            # 16KB page alignment flags
            -Wl,-z,max-page-size=16384
            -Wl,-z,common-page-size=16384
            # Additional optimization for release builds
            $<$<CONFIG:Release>:-Wl,--gc-sections>
            $<$<CONFIG:Release>:-Wl,--strip-all>)
endif()

# Define preprocessor macros if needed
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        ANDROID_PLATFORM
        # Add 16KB page awareness
        ANDROID_16KB_PAGES=1)

# REMOVED: Redundant optimization flags (now handled in target_compile_options)
# Optional: Enable optimization for release builds
# if(CMAKE_BUILD_TYPE STREQUAL "Release")
#     target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O2)
# endif()

# Optional: Add debug symbols for debug builds
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -g)
# endif()