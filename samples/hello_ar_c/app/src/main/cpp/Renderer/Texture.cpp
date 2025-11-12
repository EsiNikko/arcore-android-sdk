#include "Texture.h"
#include "Asset/AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Ray
{
    namespace
    {
        int S_ChannelsFromImageFormat(ImageFormat format)
        {
            switch (format)
            {
                // Unsigned normalized formats
                case ImageFormat::R8:       return 1;
                case ImageFormat::RG8:      return 2;
                case ImageFormat::RGB8:     return 3;
                case ImageFormat::RGBA8:    return 4;
                // Floating-point formats
                case ImageFormat::R32F:     return 1;
                case ImageFormat::RG32F:    return 2;
                case ImageFormat::RGB32F:   return 3;
                case ImageFormat::RGBA32F:  return 4;

                default:
                    RAY_CORE_ASSERT(false, "Unknown ImageFormat!");
                    return 0;
            }
        }

        ImageFormat S_ChannelsToImageFormat(int nChannels, bool hdr = false)
        {
            if (hdr)
            {
                switch (nChannels)
                {
                    case 1: return ImageFormat::R32F;
                    case 2: return ImageFormat::RG32F;
                    case 3: return ImageFormat::RGB32F;
                    case 4: return ImageFormat::RGBA32F;
                    default:
                        RAY_CORE_ASSERT(false, "Unsupported channel count!");
                        return ImageFormat::None;
                }
            }else
            {
                switch (nChannels)
                {
                    case 1: return ImageFormat::R8;
                    case 2: return ImageFormat::RG8;
                    case 3: return ImageFormat::RGB8;
                    case 4: return ImageFormat::RGBA8;
                    default:
                        RAY_CORE_ASSERT(false, "Unsupported channel count!");
                        return ImageFormat::None;
                }
            }
        }

        unsigned char* S_LoadImageWithFormat(const std::vector<uint8_t>& rawFile, TextureSpecification& specs)
        {
            int nChannels = 0;
            int width = 0, height = 0;
            unsigned char* data = nullptr;
            stbi_set_flip_vertically_on_load(true);

            switch (specs.Format)
            {
                case ImageFormat::RGBA8:
                    nChannels = 4;
                    data = stbi_load_from_memory(rawFile.data(), (int32_t)rawFile.size(), (int*)&specs.Width, (int*)&specs.Height, &nChannels, nChannels);
                    break;

            }

            RAY_CORE_VERIFY(data != nullptr, "Failed to load image!");
        }
    }
    Texture::Texture(const std::string& filePath, const TextureSpecification& specs) :
        m_Specs(specs)
    {
        std::vector<uint8_t> rawFile;
        AssetManager::ReadBinaryFile(filePath, rawFile);
        RAY_CORE_ASSERT(!rawFile.empty(), "Invalid file!");



        m_Specs.Width = width;
        m_Specs.Height = height;
        m_Specs.Format = ImageFormat::RGBA8;


        stbi_image_free(data);
    }
} // Ray