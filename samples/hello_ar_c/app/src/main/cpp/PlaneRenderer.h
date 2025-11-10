#pragma once

#include "Core/Core.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"

namespace Ray
{
    class Shader;

    // PlaneRenderer renders ARCore plane type.
    class PlaneRenderer
    {
    public:
        PlaneRenderer() = default;
        ~PlaneRenderer() = default;
        // Sets up OpenGL state used by the plane renderer.  Must be called on the
        // OpenGL thread.
        void InitializeGlContent(AAssetManager* assetManager);
        // Draws the provided plane.
        void Draw(const glm::mat4& projectionMat, const glm::mat4& viewMat,
                  const ArSession& arSession, const ArPlane& arPlane);
    private:
        void UpdateForPlane(const ArSession& arSession, const ArPlane& arPlane);

        std::vector<glm::vec3> m_vertices;
        std::vector<GLushort> m_triangles;
        glm::mat4 m_modelMat = glm::mat4(1.0f);
        glm::vec3 m_normalVec = glm::vec3(0.0f);
        GLuint m_textureId;
        Ref<Shader> m_Shader;
    };
}  // namespace Ray
