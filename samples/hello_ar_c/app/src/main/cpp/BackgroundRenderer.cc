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

// This modules handles drawing the passthrough camera image into the OpenGL
// scene.

#include "BackgroundRenderer.h"

#include <type_traits>

#include "Util.h"

namespace Ray
{
    namespace
    {
        // Positions of the quad vertices in clip space (X, Y).
        const GLfloat s_Vertices[] = {
            -1.0f, -1.0f,
            +1.0f, -1.0f,
            -1.0f, +1.0f,
            +1.0f, +1.0f,
        };
        constexpr char s_CameraVertexShaderFilename[] = "shaders/screenquad.vert";
        constexpr char s_CameraFragmentShaderFilename[] = "shaders/screenquad.frag";
        constexpr char s_DepthVisualizerVertexShaderFilename[] = "shaders/background_show_depth_color_visualization.vert";
        constexpr char s_DepthVisualizerFragmentShaderFilename[] = "shaders/background_show_depth_color_visualization.frag";
        constexpr char s_DepthColorPaletteImageFilename[] = "models/depth_color_palette.png";
    }  // namespace

    void BackgroundRenderer::InitializeGlContent(AAssetManager* assetManager, int depthTextureId)
    {
        // Defines the default background, which is the Color camera image.
        glGenTextures(1, &m_cameraTextureId);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_cameraTextureId);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        m_cameraProgram = Util::CreateProgram(s_CameraVertexShaderFilename,
                                              s_CameraFragmentShaderFilename,
                                              assetManager);
        if (!m_cameraProgram)
            LOGE("Could not create program.");

        m_cameraTextureUniform = glGetUniformLocation(m_cameraProgram, "sTexture");
        m_cameraPositionAttrib = glGetAttribLocation(m_cameraProgram, "a_Position");
        m_cameraTexcoordAttrib = glGetAttribLocation(m_cameraProgram, "a_TexCoord");

        // Defines the Color palette to use when rendering depth.
        glGenTextures(1, &m_depthColorPaletteId);
        glBindTexture(GL_TEXTURE_2D, m_depthColorPaletteId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (!Util::LoadPngFromAssetManager(GL_TEXTURE_2D, s_DepthColorPaletteImageFilename))
            LOGE("Could not load png texture for depth Color palette.");

        // Defines the depth visualization background, which shows the current depth.
        m_depthProgram = Util::CreateProgram(s_DepthVisualizerVertexShaderFilename,
                                             s_DepthVisualizerFragmentShaderFilename,
                                             assetManager);
        if (!m_depthProgram)
            LOGE("Could not create program.");

        m_depthTextureUniform = glGetUniformLocation(m_depthProgram, "u_DepthTexture");
        m_depthColorPaletteUniform = glGetUniformLocation(m_depthProgram, "u_ColorMap");
        m_depthPositionAttrib = glGetAttribLocation(m_depthProgram, "a_Position");
        m_depthTexcoordAttrib = glGetAttribLocation(m_depthProgram, "a_TexCoord");

        m_depthTextureId = depthTextureId;
    }

    void BackgroundRenderer::Draw(const ArSession* session, const ArFrame* frame, bool debugShowDepthMap)
    {
        static_assert(std::extent<decltype(s_Vertices)>::value == s_NumVertices * 2,
                "Incorrect s_Vertices length");

        // If display rotation changed (also includes view size change), we need to
        // re-query the uv coordinates for the on-screen portion of the camera image.
        int32_t geometryChanged = 0;
        ArFrame_getDisplayGeometryChanged(session, frame, &geometryChanged);
        if (geometryChanged != 0 || !m_uvsInitialized)
        {
            ArFrame_transformCoordinates2d(session, frame,
                AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
                s_NumVertices, s_Vertices,
                AR_COORDINATES_2D_TEXTURE_NORMALIZED,
                m_transformedUVs);
            m_uvsInitialized = true;
        }

        int64_t frameTimestamp;
        ArFrame_getTimestamp(session, frame, &frameTimestamp);
        if (frameTimestamp == 0)
        {
            // Suppress rendering if the camera did not produce the first frame yet.
            // This is to avoid drawing possible leftover data from previous sessions if
            // the texture is reused.
            return;
        }

        if (m_depthTextureId == -1 || m_depthColorPaletteId == -1 || m_cameraTextureId == -1)
            return;

        glDepthMask(GL_FALSE);

        if (debugShowDepthMap)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_depthColorPaletteId);
            glUseProgram(m_depthProgram);
            glUniform1i(m_depthTextureUniform, 0);
            glUniform1i(m_depthColorPaletteUniform, 1);

            // Set the vertex positions and texture coordinates.
            glVertexAttribPointer(m_depthPositionAttrib, 2, GL_FLOAT,
                                  false, 0,s_Vertices);
            glVertexAttribPointer(m_depthTexcoordAttrib, 2, GL_FLOAT,
                                  false, 0,m_transformedUVs);
            glEnableVertexAttribArray(m_depthPositionAttrib);
            glEnableVertexAttribArray(m_depthTexcoordAttrib);
        } else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_cameraTextureId);
            glUseProgram(m_cameraProgram);
            glUniform1i(m_cameraTextureUniform, 0);

            // Set the vertex positions and texture coordinates.
            glVertexAttribPointer(m_cameraPositionAttrib, 2, GL_FLOAT,
                                  false, 0,s_Vertices);
            glVertexAttribPointer(m_cameraTexcoordAttrib, 2, GL_FLOAT,
                                  false, 0,m_transformedUVs);
            glEnableVertexAttribArray(m_cameraPositionAttrib);
            glEnableVertexAttribArray(m_cameraTexcoordAttrib);
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Disable vertex arrays
        if (debugShowDepthMap)
        {
            glDisableVertexAttribArray(m_depthPositionAttrib);
            glDisableVertexAttribArray(m_depthTexcoordAttrib);
        } else
        {
            glDisableVertexAttribArray(m_cameraPositionAttrib);
            glDisableVertexAttribArray(m_cameraTexcoordAttrib);
        }

        glUseProgram(0);
        glDepthMask(GL_TRUE);
        Util::CheckGlError("BackgroundRenderer::Draw() error");
    }
}  // namespace Ray
