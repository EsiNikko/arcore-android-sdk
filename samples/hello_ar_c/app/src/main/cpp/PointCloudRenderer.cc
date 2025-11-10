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

#include "PointCloudRenderer.h"
#include "Util.h"

namespace Ray
{
    namespace
    {
        constexpr char s_VertexShaderFilename[] = "shaders/point_cloud.vert";
        constexpr char s_FragmentShaderFilename[] = "shaders/point_cloud.frag";
    }  // namespace

    void PointCloudRenderer::InitializeGlContent(AAssetManager* assetManager)
    {
        m_shaderProgram = Util::CreateProgram(s_VertexShaderFilename,
                                              s_FragmentShaderFilename,
                                              assetManager);
        if (!m_shaderProgram)
            LOGE("Could not create program.");

        m_verticesAttrib = glGetAttribLocation(m_shaderProgram, "a_Position");
        m_mvpMatUniform = glGetUniformLocation(m_shaderProgram, "u_ModelViewProjection");
        m_colorUniform = glGetUniformLocation(m_shaderProgram, "u_Color");
        m_pointSizeUniform = glGetUniformLocation(m_shaderProgram, "u_PointSize");
        Util::CheckGlError("PointCloudRenderer::InitializeGlContent()");
    }

    void PointCloudRenderer::Draw(const glm::mat4& mvpMatrix, ArSession* arSession, ArPointCloud* arPointCloud) const
    {
        CHECK(m_shaderProgram);

        glUseProgram(m_shaderProgram);

        int32_t numberOfPoints = 0;
        ArPointCloud_getNumberOfPoints(arSession, arPointCloud,
                                       &numberOfPoints);
        if (numberOfPoints <= 0)
          return;

        const float* pointCloudData;
        ArPointCloud_getData(arSession, arPointCloud, &pointCloudData);

        glUniformMatrix4fv(m_mvpMatUniform, 1, GL_FALSE,
                           glm::value_ptr(mvpMatrix));

        glEnableVertexAttribArray(m_verticesAttrib);
        glVertexAttribPointer(m_verticesAttrib, 4, GL_FLOAT, GL_FALSE, 0,
                              pointCloudData);

        // Set cyan Color to the point cloud.
        glUniform4f(m_colorUniform, 31.0f / 255.0f, 188.0f / 255.0f, 210.0f / 255.0f,1.0f);
        glUniform1f(m_pointSizeUniform, 5.0f);

        glDrawArrays(GL_POINTS, 0, numberOfPoints);

        glUseProgram(0);
        Util::CheckGlError("PointCloudRenderer::Draw");
    }
}  // namespace Ray
