/*
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ObjRenderer.h"

#include "Util.h"

namespace Ray
{
    namespace
    {
        const glm::vec4 s_LightDirection(0.0f, 1.0f, 0.0f, 0.0f);
        constexpr char s_VertexShaderFilename[] = "shaders/ar_object.vert";
        constexpr char s_FragmentShaderFilename[] = "shaders/ar_object.frag";
        constexpr char s_UseDepthForOcclusionShaderFlag[] = "USE_DEPTH_FOR_OCCLUSION";
    }  // namespace

    void ObjRenderer::InitializeGlContent(AAssetManager* assetManager,
                                          const std::string& objFileName,
                                          const std::string& pngFileName)
    {
        CompileAndLoadShaderProgram(assetManager);
        m_positionAttrib = glGetAttribLocation(m_shaderProgram, "a_Position");
        m_texcoordAttrib = glGetAttribLocation(m_shaderProgram, "a_TexCoord");
        m_normalAttrib = glGetAttribLocation(m_shaderProgram, "a_Normal");

        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (!Util::LoadPngFromAssetManager(GL_TEXTURE_2D, pngFileName.c_str()))
            LOGE("Could not load png texture for planes.");

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        Util::LoadObjFile(objFileName, assetManager, &m_vertices,
                          &m_normals, &m_uvs, &m_indices);

        Util::CheckGlError("ObjRenderer::InitializeGlContent()");
    }

    void ObjRenderer::SetUseDepthForOcclusion(AAssetManager* assetManager, bool useDepthForOcclusion)
    {
        if (m_useDepthForOcclusion == useDepthForOcclusion)
          return;  // No change, does nothing.

        // Toggles the occlusion rendering mode and recompiles the shader.
        m_useDepthForOcclusion = useDepthForOcclusion;
        CompileAndLoadShaderProgram(assetManager);
    }

    void ObjRenderer::CompileAndLoadShaderProgram(AAssetManager* assetManager)
    {
        // Compiles and loads the shader program based on the selected mode.
        std::map<std::string, int> defineValuesMap;
        defineValuesMap[s_UseDepthForOcclusionShaderFlag] = m_useDepthForOcclusion ? 1 : 0;

        m_shaderProgram = Util::CreateProgram(s_VertexShaderFilename,
                                              s_FragmentShaderFilename,
                                              assetManager,
                                              defineValuesMap);
        if (!m_shaderProgram)
            LOGE("Could not create program.");

        m_mvpMatUniform = glGetUniformLocation(m_shaderProgram, "u_ModelViewProjection");
        m_mvMatUniform = glGetUniformLocation(m_shaderProgram, "u_ModelView");
        m_textureUniform = glGetUniformLocation(m_shaderProgram, "u_Texture");

        m_lightingParamUniform = glGetUniformLocation(m_shaderProgram, "u_LightingParameters");
        m_materialParamUniform = glGetUniformLocation(m_shaderProgram, "u_MaterialParameters");
        m_colorCorrectionParamUniform = glGetUniformLocation(m_shaderProgram, "u_ColorCorrectionParameters");
        m_colorUniform = glGetUniformLocation(m_shaderProgram, "u_ObjColor");

        // Occlusion Uniforms.
        if (m_useDepthForOcclusion)
        {
            m_depthTextureUniform = glGetUniformLocation(m_shaderProgram, "u_DepthTexture");
            m_depthUvTransformUniform = glGetUniformLocation(m_shaderProgram, "u_DepthUvTransform");
            m_depthAspectratioUniform = glGetUniformLocation(m_shaderProgram, "u_DepthAspectRatio");
        }
    }

    void ObjRenderer::SetMaterialProperty(float ambient, float diffuse, float specular, float specularPower)
    {
        m_ambient = ambient;
        m_diffuse = diffuse;
        m_specular = specular;
        m_specularPower = specularPower;
    }

    void ObjRenderer::Draw(const glm::mat4& projectionMat,
                           const glm::mat4& viewMat, const glm::mat4& modelMat,
                           const float* colorCorrection4,
                           const float* objectColor4) const
    {
        if (!m_shaderProgram)
        {
            LOGE("shader program is null.");
            return;
        }

        glUseProgram(m_shaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(m_textureUniform, 0);
        glBindTexture(GL_TEXTURE_2D, m_textureId);

        glm::mat4 mvp_mat = projectionMat * viewMat * modelMat;
        glm::mat4 mv_mat = viewMat * modelMat;
        glm::vec4 view_light_direction = glm::normalize(mv_mat * s_LightDirection);

        glUniform4f(m_lightingParamUniform, view_light_direction[0],
                    view_light_direction[1], view_light_direction[2], 1.f);
        glUniform4f(m_materialParamUniform, m_ambient, m_diffuse, m_specular,
                    m_specularPower);
        glUniform4fv(m_colorCorrectionParamUniform, 1, colorCorrection4);
        glUniform4fv(m_colorUniform, 1, objectColor4);

        glUniformMatrix4fv(m_mvpMatUniform, 1, GL_FALSE,
                           glm::value_ptr(mvp_mat));
        glUniformMatrix4fv(m_mvMatUniform, 1, GL_FALSE,
                           glm::value_ptr(mv_mat));

        // Occlusion parameters.
        if (m_useDepthForOcclusion) {
            // Attach the depth texture.
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
            glUniform1i(m_depthTextureUniform, 1);

            // Set the depth texture uv transform.
            glUniformMatrix3fv(m_depthUvTransformUniform, 1, GL_FALSE,
                               glm::value_ptr(m_uvTransform));
            glUniform1f(m_depthAspectratioUniform, m_depthAspectratio);
        }

        // Note: for simplicity, we are uploading the model each time we draw it.  A
        // real application should use vertex buffers to upload the geometry once.
        glEnableVertexAttribArray(m_positionAttrib);
        glVertexAttribPointer(m_positionAttrib, 3, GL_FLOAT, GL_FALSE, 0,
                              m_vertices.data());

        glEnableVertexAttribArray(m_normalAttrib);
        glVertexAttribPointer(m_normalAttrib, 3, GL_FLOAT, GL_FALSE, 0,
                              m_normals.data());

        glEnableVertexAttribArray(m_texcoordAttrib);
        glVertexAttribPointer(m_texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 0,
                              m_uvs.data());

        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);

        // Textures are loaded with premultiplied alpha
        // (https://developer.android.com/reference/android/graphics/BitmapFactory.Options#inPremultiplied),
        // so we use the premultiplied alpha blend factors.
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_SHORT,
                       m_indices.data());

        glDisable(GL_BLEND);
        glDisableVertexAttribArray(m_positionAttrib);
        glDisableVertexAttribArray(m_texcoordAttrib);
        glDisableVertexAttribArray(m_normalAttrib);

        glUseProgram(0);
        Util::CheckGlError("ObjRenderer::Draw()");
    }

    void ObjRenderer::SetDepthTexture(int textureId, int width, int height)
    {
        m_depthTextureId = textureId;
        m_depthAspectratio = (float)width / (float)height;
    }
}  // namespace Ray
