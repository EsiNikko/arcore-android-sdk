#pragma once
#include "Core/Core.h"

namespace Ray
{
    enum class ImageFormat : uint8_t
    {
        None = 0,
        // Unsigned normalized formats
        R8, RG8, RGB8, RGBA8,
        // Floating-point formats
        R32F, RG32F, RGB32F, RGBA32F,
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

    class Texture
    {
    public:
        Texture(const std::string& filePath, const TextureSpecification& specs);
        ~Texture();

    private:
        TextureSpecification m_Specs;
    };

} // Ray

