# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the lib name from manifest file. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("NDKNativeAppGlue") ## ARTR_TS: Declares the lib name from manifest file. The project name can be accessed via ${ PROJECT_NAME}, and since this is the top level CMakeLists.txt, the project name is also accessible with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level build script scope).

## ARTR_TS: Now this is mandatory otherwise apllication will crash
## ARTR_TS: Following line is important to let andriod understand the entry point function for linker
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate") # -u: Unconditionally include the specified symbol in the output file, even if it is not referenced by any other symbol. This is necessary for the entry point function of a NativeActivity, which may not be explicitly referenced in the code but is required for the application to run correctly.

## ARTR_TS: The following 2 lines tell CMAKE to include Android's native app glue wrapper code, which provides the necessary glue code to connect the Android framework with native C/C++ code. This is essential for applications that use NativeActivity, as it allows the application to receive lifecycle events and input events from the Android system.
include_directories("C:\\Users\\HI\\AppData\\Local\\Android\\Sdk\\cmake\\3.22.1\\share\\cmake-3.22\\Modules")
include("C:\\Users\\HI\\AppData\\Local\\Android\\Sdk\\cmake\\3.22.1\\share\\cmake-3.22\\Modules\\AndroidNdkModules.cmake") ## ARTR_TS: This line includes the android-ndk-modules.cmake file, which is part of the Android NDK and provides functions for importing Android system libraries and modules.
android_ndk_import_module_native_app_glue()

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
## ARTR_TS: SHARED=Define the library as a shared library, which is necessary for Android applications that use native code, as it allows the library to be loaded at runtime by the Android system.
add_library(${CMAKE_PROJECT_NAME} ## ARTR_TS: Add project name as library name, which is defined in the project() command above
    SHARED 
    ${CMAKE_SOURCE_DIR}/VK.cpp) ## ARTR_TS: Add the source file for the native library. The path is relative to the CMakeLists.txt file, and it points to the window.cpp file in the src/main/cpp directory.
    # List C/C++ source files with relative paths to this CMakeLists.txt.

add_library(vulkan-validation-lib
            SHARED
            IMPORTED)

set_target_properties(vulkan-validation-lib
                    PROPERTIES IMPORTED_LOCATION
                    ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libVkLayer_khronos_validation.so)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
    # List libraries link to the target library
    android
    vulkan
    vulkan-validation-lib
    native_app_glue
    log
    ) ## ARTR_TS: Link the native_app_glue library, which is necessary for applications that use NativeActivity, as it provides the necessary glue code to connect the Android framework with native C/C++ code.
