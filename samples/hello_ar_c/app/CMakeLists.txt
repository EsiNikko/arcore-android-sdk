# Copyright 2017 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

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

# Import the glm header file from the NDK.
add_library( glm INTERFACE )
set_target_properties( glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE})

# This is the main app library.
add_library(hello_ar_native SHARED
        src/main/cpp/Core/Timer.cc
        src/main/cpp/Core/Log.cc
        src/main/cpp/Asset/AssetManager.cpp
        src/main/cpp/Renderer/Shader.cc
        src/main/cpp/BackgroundRenderer.cc
        src/main/cpp/RayArApplication.cc
        src/main/cpp/JniInterface.cc
        src/main/cpp/ObjRenderer.cc
        src/main/cpp/PlaneRenderer.cc
        src/main/cpp/PointCloudRenderer.cc
        src/main/cpp/Texture.cc
        src/main/cpp/Util.cc)

target_include_directories(hello_ar_native PRIVATE
        src/main/cpp)
target_link_libraries(hello_ar_native
                      android
                      log
                      GLESv2
                      glm
                      arcore)
