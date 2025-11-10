/*
 * Copyright 2020 Google LLC
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

#include "Texture.h"

// clang-format off
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>
// clang-format on
#include "Util.h"

namespace Ray
{
    void Texture::CreateOnGlThread()
    {
        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void Texture::UpdateWithDepthImageOnGlThread(const ArSession& session, const ArFrame& frame)
    {
        ArImage* depthImage = nullptr;
        if (ArFrame_acquireDepthImage16Bits(&session, &frame, &depthImage) != AR_SUCCESS)
        {
            // No depth image received for this frame.
            return;
        }

        // Checks that the format is as expected.
        ArImageFormat imageFormat;
        ArImage_getFormat(&session, depthImage, &imageFormat);
        if (imageFormat != AR_IMAGE_FORMAT_D_16)
        {
            LOGE("Unexpected image format 0x%x", imageFormat);
            ArImage_release(depthImage);
            abort();
            return;
        }

        const uint8_t* depthData = nullptr;
        int planeSizeBytes = 0;
        ArImage_getPlaneData(&session, depthImage, /*plane_index=*/0, &depthData,
                             &planeSizeBytes);

        // Bails out if there's no depthData.
        if (depthData == nullptr)
        {
            ArImage_release(depthImage);
            return;
        }

        // Sets texture sizes.
        int image_width = 0;
        int image_height = 0;
        int image_pixel_stride = 0;
        int image_row_stride = 0;
        ArImage_getWidth(&session, depthImage, &image_width);
        ArImage_getHeight(&session, depthImage, &image_height);
        ArImage_getPlanePixelStride(&session, depthImage, 0, &image_pixel_stride);
        ArImage_getPlaneRowStride(&session, depthImage, 0, &image_row_stride);
        ArImage_release(depthImage);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, image_width,
                     image_height, 0, GL_RG,GL_UNSIGNED_BYTE, depthData);
        m_width = image_width;
        m_height = image_height;
    }
}  // namespace Ray
