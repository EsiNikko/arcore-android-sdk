#pragma once
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"

namespace Ray
{
    // PlaneRenderer renders ARCore plane type.
    class ObjRenderer
    {
    public:
        ObjRenderer() = default;
        ~ObjRenderer() = default;
        // Loads the OBJ file and texture and sets up OpenGL resources used to draw
        // the model.  Must be called on the OpenGL thread prior to any other calls.
        void InitializeGlContent(AAssetManager* assetManager,
                               const std::string& objFileName,
                               const std::string& pngFileName);
        // Sets the surface's lighting reflectace properties.  Diffuse is modulated by
        // the texture's Color.
        void SetMaterialProperty(float ambient, float diffuse, float specular, float specularPower);
        // Draws the model.
        void Draw(const glm::mat4& projectionMat, const glm::mat4& viewMat,
                  const glm::mat4& modelMat, const float* colorCorrection4,
                  const float* objectColor4) const;
        void SetUvTransformMatrix(const glm::mat3& uvTransform) { m_uvTransform = uvTransform; }
        void SetDepthTexture(int textureId, int width, int height);
        // Specifies whether to use the depth texture to perform depth-based occlusion
        // of virtual objects from real-world geometry.
        //
        // This function is a no-op if the value provided is the same as what is
        // already set. If the value changes, this function will recompile and reload
        // the shader program to either enable/disable depth-based occlusion. NOTE:
        // recompilation of the shader is inefficient. This code could be optimized to
        // precompile both versions of the shader.
        //
        // @param context Context for loading the shader.
        // @param useDepthForOcclusion Specifies whether to use the depth texture to
        // perform occlusion during rendering of virtual objects.
        void SetUseDepthForOcclusion(AAssetManager* assetManager, bool useDepthForOcclusion);

    private:
        void CompileAndLoadShaderProgram(AAssetManager* assetManager);

        // Shader material lighting pateremrs
        float m_ambient = 0.0f;
        float m_diffuse = 2.0f;
        float m_specular = 0.5f;
        float m_specularPower = 6.0f;

        // Model attribute arrays
        std::vector<GLfloat> m_vertices;
        std::vector<GLfloat> m_uvs;
        std::vector<GLfloat> m_normals;
        // Model triangle indices
        std::vector<GLushort> m_indices;
        // Loaded TEXTURE_2D object name
        GLuint m_textureId;
        GLuint m_depthTextureId;
        // Shader program details
        GLuint m_shaderProgram;
        GLint m_positionAttrib;
        GLint m_texcoordAttrib;
        GLint m_normalAttrib;
        GLint m_mvpMatUniform;
        GLint m_mvMatUniform;
        GLint m_textureUniform;
        GLint m_lightingParamUniform;
        GLint m_materialParamUniform;
        GLint m_colorCorrectionParamUniform;
        GLint m_colorUniform;
        GLint m_depthTextureUniform;
        GLint m_depthUvTransformUniform;
        GLint m_depthAspectratioUniform;

        bool m_useDepthForOcclusion = false;
        float m_depthAspectratio = 0.0f;
        glm::mat3 m_uvTransform = glm::mat3(1.0f);
    };
}  // namespace Ray
