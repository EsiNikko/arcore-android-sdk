#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <cstdlib>
#include <vector>
#include "arcore_c_api.h"
#include "glm.h"

namespace Ray
{
    class PointCloudRenderer
    {
    public:
        // Default constructor of PointCloudRenderer.
        PointCloudRenderer() = default;
        // Default deconstructor of PointCloudRenderer.
        ~PointCloudRenderer() = default;

        // Initialize the GL content, needs to be called on GL thread.
        void InitializeGlContent(AAssetManager* assetManager);
        // Render the AR point cloud.
        //
        // @param mvpMatrix, the model view projection matrix of point cloud.
        // @param arSession, the session that is used to query point cloud points from arPointCloud.
        // @param arPointCloud, point cloud data to for rendering.
        void Draw(const glm::mat4& mvpMatrix, ArSession* arSession, ArPointCloud* arPointCloud) const;

    private:
        GLuint m_shaderProgram;
        GLint m_verticesAttrib;
        GLint m_mvpMatUniform;
        GLint m_colorUniform;
        GLint m_pointSizeUniform;
    };
}  // namespace Ray
