#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <errno.h>
#include <jni.h>

#include <cstdint>
#include <cstdlib>
#include <map>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"

#ifndef LOGI
#define LOGI(...) \
    __android_log_print(ANDROID_LOG_INFO, "RayAR_C++", __VA_ARGS__)
#endif  // LOGI

#ifndef LOGE
#define LOGE(...) \
    __android_log_print(ANDROID_LOG_ERROR, "RayAR_C++", __VA_ARGS__)
#endif  // LOGE

#ifndef CHECK
#define CHECK(condition)                                                        \
    if (!(condition)) {                                                         \
        LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition);  \
        abort();                                                                \
    }
#endif  // CHECK

#ifndef CHECK_AND_THROW
#define CHECK_AND_THROW(condition, env, msg, ...)                               \
    if (!(condition))                                                           \
    {                                                                           \
        LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition);  \
        Util::ThrowJavaException(env, msg);                                     \
        return ##__VA_ARGS__;                                                   \
    }                                                                           \
    do{}while(false)
#endif  // CHECK_AND_THROW

namespace Ray
{
    // Utilities for C hello AR project.
    namespace Util
    {
        // Provides a scoped allocated instance of Anchor.
        // Can be treated as an ArAnchor*.
        class ScopedArPose
        {
        public:
            explicit ScopedArPose(const ArSession* session)
            {
                ArPose_create(session, nullptr, &m_pose);
            }
            ~ScopedArPose() { ArPose_destroy(m_pose); }
            ArPose* GetArPose() { return m_pose; }
            // Delete copy constructors.
            ScopedArPose(const ScopedArPose&) = delete;
            void operator=(const ScopedArPose&) = delete;

        private:
            ArPose* m_pose = nullptr;
        };

        // Check GL error, and abort if an error is encountered.
        //
        // @param operation, the name of the GL function call.
        void CheckGlError(const char* operation);

        // Throw a Java exception.
        //
        // @param env, the JNIEnv.
        // @param msg, the message of this exception.
        void ThrowJavaException(JNIEnv* env, const char* msg);

        // Create a shader program ID.
        //
        // @param assetManager, AAssetManager pointer.
        // @param vertexShaderFileName, the vertex shader source file.
        // @param fragmentShaderFileName, the fragment shader source file.
        // @return a non-zero value if the shader is created successfully, otherwise 0.
        GLuint CreateProgram(const char* vertexShaderFileName,
                             const char* fragmentShaderFileName,
                             AAssetManager* assetManager);

        // Create a shader program ID.
        //
        // @param assetManager, AAssetManager pointer.
        // @param vertexShaderFileName, the vertex shader source file.
        // @param fragmentShaderFileName, the fragment shader source file.
        // @param defineValuesMap The #define values to add to the top of the shader
        // source code.
        // @return a non-zero value if the shader is created successfully, otherwise 0.
        GLuint CreateProgram(const char* vertexShaderFileName,
                             const char* fragmentShaderFileName,
                             AAssetManager* assetManager,
                             const std::map<std::string, int>& defineValuesMap);

        // Load a text file from assets folder.
        //
        // @param assetManager, AAssetManager pointer.
        // @param fileName, path to the file, relative to the assets folder.
        // @param out_string, output string.
        // @return true if the file is loaded correctly, otherwise false.
        bool LoadTextFileFromAssetManager(const char* fileName,
                                          AAssetManager* assetManager,
                                          std::string* outFileTextString);

        // Load png file from assets folder and then assign it to the OpenGL target.
        // This method must be called from the renderer thread since it will result in
        // OpenGL calls to assign the image to the texture target.
        //
        // @param target, openGL texture target to load the image into.
        // @param path, path to the file, relative to the assets folder.
        // @return true if png is loaded correctly, otherwise false.
        bool LoadPngFromAssetManager(int target, const char* path);

        // Load obj file from assets folder from the app.
        //
        // @param assetManager, AAssetManager pointer.
        // @param fileName, name of the obj file.
        // @param outVertices, output vertices.
        // @param outNormals, output normals.
        // @param outUVs, output texture UV coordinates.
        // @param outIndices, output triangle indices.
        // @return true if obj is loaded correctly, otherwise false.
        bool LoadObjFile(const std::string& fileName,
                         AAssetManager* assetManager,
                         std::vector<GLfloat>* outVertices,
                         std::vector<GLfloat>* outNormals,
                         std::vector<GLfloat>* outUVs,
                         std::vector<GLushort>* outIndices);

        // Format and output the matrix to logcat file.
        // Note that this function output matrix in row major.
        void Log4x4Matrix(const float rawMatrix[16]);

        // Get transformation matrix from ArAnchor.
        void GetTransformMatrixFromAnchor(const ArAnchor& arAnchor,
                                          ArSession* arSession,
                                          glm::mat4* outModelMat);

        // Get the plane's normal from center pose.
        glm::vec3 GetPlaneNormal(const ArSession& arSession, const ArPose& planePose);

        // Calculate the normal distance to plane from cameraPose, the given planePose
        // should have y axis parallel to plane's normal, for example plane's center
        // pose or hit test pose.
        float CalculateDistanceToPlane(const ArSession& arSession,
                                       const ArPose& planePose,
                                       const ArPose& cameraPose);
    }  // namespace Util
}  // namespace Ray
