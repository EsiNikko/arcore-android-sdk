#pragma once
#include "Core/Core.h"
#include "Core/RawBuffer.h"

namespace Ray
{
    enum class ImageFormat : uint8_t
    {
        None = 0,
        // 8-bit UNORM color
        R8, RG8, RGB8, RGBA8,
        // Half-float / HDR
        R16F, RG16F, RGB16F, RGBA16F,
        // Full precision float
        R32F, RG32F, RGB32F, RGBA32F,
        // sRGB
        SRGB8, SRGB8_ALPHA8,
        // Depth formats
        DEPTH24, DEPTH24_STENCIL8
    };
    enum class ImageWrapMode : uint8_t
    {
        None	= 0,
        Repeat	= 1,
        Clamp	= 2,
        Mirror	= 3,
    };
    enum class ImageFilter : uint8_t
    {
        None	= 0,
        Linear	= 1,
        Nearest = 2,
    };
    struct TextureSpecification
    {
        uint32_t		Width = 1;
        uint32_t		Height = 1;
        ImageFormat		Format = ImageFormat::RGBA8;
        ImageWrapMode	WrapMode = ImageWrapMode::Repeat;
        ImageFilter		MinFilter = ImageFilter::Linear;
        ImageFilter		MagFilter = ImageFilter::Linear;
        bool			GenerateMips = true;
        bool            IsHDR = false;
    };

    class Texture2D
    {
    public:
        static Ref<Texture2D> Create(const std::string& filePath);
        static Ref<Texture2D> Create(const std::string& filePath, const TextureSpecification& specs);

        explicit Texture2D(const RawBuffer& buffer, const TextureSpecification& specs);
        ~Texture2D();

    private:
        TextureSpecification m_Specs;
    };

} // Ray

