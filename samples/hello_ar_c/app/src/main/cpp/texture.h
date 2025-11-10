#pragma once

#include <arcore_c_api.h>

namespace Ray
{
    /// Handle the creation and update of a GPU texture.
    class Texture
    {
    public:
        Texture() = default;
        ~Texture() = default;

        void CreateOnGlThread();
        void UpdateWithDepthImageOnGlThread(const ArSession& session, const ArFrame& frame);
        unsigned int GetTextureId() const { return m_textureId; }
        unsigned int GetWidth() const { return m_width; }
        unsigned int GetHeight() const { return m_height; }
    private:
        unsigned int m_textureId = 0;
        unsigned int m_width = 1;
        unsigned int m_height = 1;
    };
}  // namespace Ray
