#include "Texture2D.h"
#include "Asset/AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Ray
{
    namespace
    {
        constexpr bool k_SetFlipVerticallyOnLoad = true;

        int S_ChannelsFromImageFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::R8:
                case ImageFormat::R16F:
                case ImageFormat::R32F: return 1;

                case ImageFormat::RG8:
                case ImageFormat::RG16F:
                case ImageFormat::RG32F: return 2;

                case ImageFormat::RGB8:
                case ImageFormat::RGB16F:
                case ImageFormat::RGB32F:
                case ImageFormat::SRGB8: return 3;

                case ImageFormat::RGBA8:
                case ImageFormat::RGBA16F:
                case ImageFormat::RGBA32F:
                case ImageFormat::SRGB8_ALPHA8: return 4;

                default:
                    RAY_CORE_ASSERT(false, "Unknown ImageFormat!");
                    return 0;
            }
        }
        ImageFormat S_ChannelsToImageFormat(int nChannels, bool isHDR, bool srgb = false)
        {
            if (srgb)
            {
                switch (nChannels)
                {
                    case 3: return ImageFormat::SRGB8;
                    case 4: return ImageFormat::SRGB8_ALPHA8;
                    default:
                        RAY_CORE_ASSERT(false, "Unsupported sRGB channel count!");
                        return ImageFormat::None;
                }
            }

            if (isHDR)
            {
                switch (nChannels)
                {
                    case 1: return ImageFormat::R16F;
                    case 2: return ImageFormat::RG16F;
                    case 3: return ImageFormat::RGB16F;
                    case 4: return ImageFormat::RGBA16F;
                    default:
                        RAY_CORE_ASSERT(false, "Unsupported HDR channel count!");
                        return ImageFormat::None;
                }
            }
            else
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

        bool S_LoadImageData(const std::vector<uint8_t>& rawFile, TextureSpecification& outSpecs, RawBuffer &outBuffer)
        {
            stbi_set_flip_vertically_on_load(k_SetFlipVerticallyOnLoad);

            int width = 0, height = 0, nChannels = 0;
            bool isHDR = stbi_is_hdr_from_memory((const stbi_uc*)rawFile.data(), (int32_t)rawFile.size());
            if (isHDR)
            {
                float* data = stbi_loadf_from_memory((const stbi_uc*)rawFile.data(),(int32_t)rawFile.size(),
                                                     &width,&height,&nChannels, 0);
                if (data == nullptr)
                {
                    RAY_CORE_ERROR("Failed to load hdr texture!");
                    return false;
                }

                outBuffer.Release();
                outBuffer = RawBuffer::Copy(data, sizeof(float) * width * height * nChannels);
                stbi_image_free(data);
            }else
            {
                stbi_uc* data = stbi_load_from_memory((const stbi_uc*)rawFile.data(),(int32_t)rawFile.size(),
                                                      &width,&height, &nChannels, 0);
                if (data == nullptr)
                {
                    RAY_CORE_ERROR("Failed to load hdr texture!");
                    return false;
                }

                outBuffer.Release();
                outBuffer = RawBuffer::Copy(data, sizeof(stbi_uc) * width * height * nChannels);
                stbi_image_free(data);
            }

            outSpecs.Width = width;
            outSpecs.Height = height;
            outSpecs.IsHDR = isHDR;
            outSpecs.Format = S_ChannelsToImageFormat(nChannels, isHDR, false);
            return true;
        }

        bool S_LoadImageDataWithSpecs(const std::vector<uint8_t>& rawFile, const TextureSpecification& specs, RawBuffer &outBuffer)
        {
            stbi_set_flip_vertically_on_load(k_SetFlipVerticallyOnLoad);

            int width = 0, height = 0, nChannels = 0;
            int desiredChannels = S_ChannelsFromImageFormat(specs.Format);

            if (specs.IsHDR)
            {
                float* data = stbi_loadf_from_memory((const stbi_uc*)rawFile.data(),(int32_t)rawFile.size(),
                                                     &width, &height, &nChannels,
                                                     desiredChannels // force channel count based on specs
                );

                if (data == nullptr)
                {
                    RAY_CORE_ERROR("Failed to load HDR texture!");
                    return false;
                }

                outBuffer.Release();
                outBuffer = RawBuffer::Copy(data, sizeof(float) * width * height * desiredChannels);
                stbi_image_free(data);
            }
            else
            {
                stbi_uc* data = stbi_load_from_memory((const stbi_uc*)rawFile.data(),(int32_t)rawFile.size(),
                                                      &width, &height, &nChannels,
                                                      desiredChannels // force channel count based on specs
                );

                if (data == nullptr)
                {
                    RAY_CORE_ERROR("Failed to load LDR texture!");
                    return false;
                }

                outBuffer.Release();
                outBuffer = RawBuffer::Copy(data, sizeof(stbi_uc) * width * height * desiredChannels);
                stbi_image_free(data);
            }

            // Optional sanity check: warn if file size doesn’t match spec
            if (width != specs.Width || height != specs.Height)
            {
                RAY_CORE_WARN("Image size differs from TextureSpecification (file: {}x{}, spec: {}x{})",
                              width, height, specs.Width, specs.Height);
            }
            return true;
        }
    }

    Texture2D::Texture2D(const RawBuffer& buffer, const TextureSpecification& specs) :
        m_Specs(specs)
    {
        RAY_CORE_ASSERT(buffer.Size > 0, "Attempted to create a Texture2D with invalid buffer!");
        RAY_CORE_TRACE("Create Texture2D: {}x{}.", specs.Width, specs.Height);
    }

    Ref<Texture2D> Texture2D::Create(const std::string &filePath)
    {
        std::vector<uint8_t> rawFile;
        AssetManager::ReadBinaryFile(filePath, rawFile);
        RAY_CORE_ASSERT(!rawFile.empty(), "Invalid file!");

        RawBuffer buffer;
        TextureSpecification specs;
        if (!S_LoadImageData(rawFile, specs, buffer)) return nullptr;

        auto texture= CreateRef<Texture2D>(buffer, specs);
        buffer.Release();
        return texture;
    }

    Ref<Texture2D> Texture2D::Create(const std::string &filePath, const TextureSpecification &specs)
    {
        std::vector<uint8_t> rawFile;
        AssetManager::ReadBinaryFile(filePath, rawFile);
        RAY_CORE_ASSERT(!rawFile.empty(), "Invalid file!");

        RawBuffer buffer;
        if (!S_LoadImageDataWithSpecs(rawFile, specs, buffer)) return nullptr;

        auto texture= CreateRef<Texture2D>(buffer, specs);
        buffer.Release();
    }

    Texture2D::~Texture2D()
    {
        RAY_CORE_TRACE("Create Texture2D: {}x{}.", m_Specs.Width, m_Specs.Height);
    }
} // Ray