# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

# Use c++_shared if you have multiple native libraries or AAR dependencies
# Use c++_static (the NDK/CMake default) if you only have one native library
set(ANDROID_STL "c++_shared")

# (Optional but Recommended) Set the C++ language standard for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Import the ARCore (Google Play Services for AR) library.
add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION
                      ${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so
                      INTERFACE_INCLUDE_DIRECTORIES ${ARCORE_INCLUDE}
)

# Import the glm header file from the third_party libraries.
add_library( glm INTERFACE )
set_target_properties( glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE})
# Import the stb header file from the third_party libraries.
add_library( stb INTERFACE )
set_target_properties( stb PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${STB_INCLUDE})

# This is the main app library.
add_library(hello_ar_native SHARED
        src/main/cpp/Core/Timer.cpp
        src/main/cpp/Core/Log.cpp
        src/main/cpp/Asset/AssetManager.cpp
        src/main/cpp/Renderer/Shader.cpp
        src/main/cpp/Renderer/Texture2D.cpp
        src/main/cpp/BackgroundRenderer.cpp
        src/main/cpp/RayArApplication.cpp
        src/main/cpp/JniInterface.cpp
        src/main/cpp/ObjRenderer.cpp
        src/main/cpp/PlaneRenderer.cpp
        src/main/cpp/PointCloudRenderer.cpp
        src/main/cpp/Texture.cpp
        src/main/cpp/Util.cpp
)

target_include_directories(hello_ar_native PRIVATE
        src/main/cpp
)
target_link_libraries(hello_ar_native
        android
        log
        GLESv2
        glm
        stb
        arcore
)
