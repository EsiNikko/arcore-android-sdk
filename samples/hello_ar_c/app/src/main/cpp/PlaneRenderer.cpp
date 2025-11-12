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

#include "PlaneRenderer.h"
#include "Renderer/Shader.h"
#include <string>
#include "Util.h"

namespace Ray
{
    void PlaneRenderer::InitializeGlContent(AAssetManager* assetManager)
    {
        m_Shader = CreateRef<Shader>("shaders/plane.vert",
                                     "shaders/plane.frag");

        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (!Util::LoadPngFromAssetManager(GL_TEXTURE_2D, "models/trigrid.png"))
            LOGE("Could not load png texture for planes.");

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        Util::CheckGlError("plane_renderer::InitializeGlContent()");
    }

    void PlaneRenderer::Draw(const glm::mat4& projectionMat,
                             const glm::mat4& viewMat, const ArSession& arSession,
                             const ArPlane& arPlane)
    {
        UpdateForPlane(arSession, arPlane);

        m_Shader->Bind();
        glDepthMask(GL_FALSE);

        glActiveTexture(GL_TEXTURE0);
        //glUniform1i(m_textureUniform, 0);
        m_Shader->SetInt("u_Texture", 0);
        glBindTexture(GL_TEXTURE_2D, m_textureId);

        // Compose final mvp matrix for this plane renderer.
        //glUniformMatrix4fv(m_mvpMatUniform, 1, GL_FALSE,
        //                   glm::value_ptr(projectionMat * viewMat * m_modelMat));
        m_Shader->SetMat4("u_ModelViewProjection", projectionMat * viewMat * m_modelMat);
        //glUniformMatrix4fv(m_mvMatUniform, 1, GL_FALSE,
        //                   glm::value_ptr(viewMat * m_modelMat));
        m_Shader->SetMat4("u_ModelView", viewMat * m_modelMat);
        //glUniformMatrix4fv(m_modelMatUniform, 1, GL_FALSE,
        //                   glm::value_ptr(m_modelMat));
        m_Shader->SetMat4("u_Model", m_modelMat);
        //glUniform3f(m_normalVecUniform, m_normalVec.x, m_normalVec.y, m_normalVec.z);
        m_Shader->SetVec3("u_Normal", m_normalVec);


        //glEnableVertexAttribArray(m_verticesAttrib);
        //glVertexAttribPointer(m_verticesAttrib, 3, GL_FLOAT, GL_FALSE,
        //                      0,m_vertices.data());
        m_Shader->SetVertexAttribFloatPointer("a_Vertex", 3, false, 0, m_vertices.data());

        glEnable(GL_BLEND);

        // Textures are loaded with premultiplied alpha
        // (https://developer.android.com/reference/android/graphics/BitmapFactory.Options#inPremultiplied),
        // so we use the premultiplied alpha blend factors.
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES, m_triangles.size(), GL_UNSIGNED_SHORT,
                       m_triangles.data());

        glDisable(GL_BLEND);
        //glUseProgram(0);
        m_Shader->Unbind();
        glDepthMask(GL_TRUE);
        Util::CheckGlError("PlaneRenderer::Draw()");
    }

    void PlaneRenderer::UpdateForPlane(const ArSession& arSession, const ArPlane& arPlane)
    {
        // The following code generates a triangle mesh filling a convex polygon,
        // including a feathered edge for blending.
        //
        // The indices shown in the diagram are used in comments below.
        // _______________     0_______________1
        // |             |      |4___________5|
        // |             |      | |         | |
        // |             | =>   | |         | |
        // |             |      | |         | |
        // |             |      |7-----------6|
        // ---------------     3---------------2

        m_vertices.clear();
        m_triangles.clear();

        int32_t polygonLength;
        ArPlane_getPolygonSize(&arSession, &arPlane, &polygonLength);

        if (polygonLength == 0)
        {
            LOGE("PlaneRenderer::UpdatePlane, no valid plane polygon is found");
            return;
        }

        const int32_t verticesSize = polygonLength / 2;
        std::vector<glm::vec2> rawVertices(verticesSize);
        ArPlane_getPolygon(&arSession, &arPlane,
                           glm::value_ptr(rawVertices.front()));

        // Fill vertex 0 to 3. Note that the vertex.xy are used for x and z
        // position. vertex.z is used for alpha. The outer polygon's alpha is 0.
        for (int32_t i = 0; i < verticesSize; ++i)
            m_vertices.emplace_back(rawVertices[i], 0.0f);

        Util::ScopedArPose scopedArPose(&arSession);
        ArPlane_getCenterPose(&arSession, &arPlane, scopedArPose.GetArPose());
        ArPose_getMatrix(&arSession, scopedArPose.GetArPose(),
                         glm::value_ptr(m_modelMat));
        m_normalVec = Util::GetPlaneNormal(arSession, *scopedArPose.GetArPose());

        // Feather distance 0.2 meters.
        const float kFeatherLength = 0.2f;
        // Feather scale over the distance between plane center and vertices.
        const float kFeatherScale = 0.2f;

        // Fill vertex 4 to 7, with alpha set to 1.
        for (int32_t i = 0; i < verticesSize; ++i)
        {
            // Vector from plane center to current point.
            glm::vec2 v = rawVertices[i];
            const float scale = 1.0f - std::min((kFeatherLength / glm::length(v)), kFeatherScale);
            const glm::vec2 resultV = scale * v;
            m_vertices.emplace_back(resultV, 1.0f);
        }

        const int32_t verticesLength = m_vertices.size();
        const int32_t halfVerticesLength = verticesLength / 2;

        // Generate triangle (4, 5, 6) and (4, 6, 7).
        for (int i = halfVerticesLength + 1; i < verticesLength - 1; ++i)
        {
            m_triangles.push_back(halfVerticesLength);
            m_triangles.push_back(i);
            m_triangles.push_back(i + 1);
        }

        // Generate triangle (0, 1, 4), (4, 1, 5), (5, 1, 2), (5, 2, 6),
        // (6, 2, 3), (6, 3, 7), (7, 3, 0), (7, 0, 4)
        for (int i = 0; i < halfVerticesLength; ++i)
        {
            m_triangles.push_back(i);
            m_triangles.push_back((i + 1) % halfVerticesLength);
            m_triangles.push_back(i + halfVerticesLength);

            m_triangles.push_back(i + halfVerticesLength);
            m_triangles.push_back((i + 1) % halfVerticesLength);
            m_triangles.push_back((i + halfVerticesLength + 1) % halfVerticesLength + halfVerticesLength);
        }
    }
}  // namespace Ray
