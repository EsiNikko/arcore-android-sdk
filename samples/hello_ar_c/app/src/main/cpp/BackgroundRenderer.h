#pragma once
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <cstdlib>

#include "arcore_c_api.h"
#include "Util.h"

namespace Ray
{
    // This class renders the passthrough camera image into the OpenGL frame.
    class BackgroundRenderer
    {
    public:
        BackgroundRenderer() = default;
        ~BackgroundRenderer() = default;

        // Sets up OpenGL state.  Must be called on the OpenGL thread and before any
        // other methods below.
        void InitializeGlContent(AAssetManager* assetManager, int depthTextureId);
        // Draws the background image.  This methods must be called for every ArFrame
        // returned by ArSession_update() to catch display geometry change events.
        //  debugShowDepthMap Toggles whether to show the live camera feed or latest
        //  depth image.
        void Draw(const ArSession* session, const ArFrame* frame, bool debugShowDepthMap);
        // Returns the generated texture name for the GL_TEXTURE_EXTERNAL_OES target.
        GLuint GetTextureId() const { return m_cameraTextureId; };

    private:
        static constexpr int s_NumVertices = 4;

        GLuint m_cameraProgram;
        GLuint m_depthProgram;

        GLuint m_cameraTextureId;
        GLuint m_depthTextureId;
        GLuint m_depthColorPaletteId;

        GLuint m_cameraPositionAttrib;
        GLuint m_cameraTexcoordAttrib;
        GLuint m_cameraTextureUniform;

        GLuint m_depthTextureUniform;
        GLuint m_depthColorPaletteUniform;
        GLuint m_depthPositionAttrib;
        GLuint m_depthTexcoordAttrib;

        float m_transformedUVs[s_NumVertices * 2];
        bool m_uvsInitialized = false;
    };
}  // namespace Ray
