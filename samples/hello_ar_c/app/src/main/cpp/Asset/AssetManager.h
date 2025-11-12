#pragma once
#include "Core/Core.h"

struct AAssetManager;

namespace Ray
{
    class AssetManager
    {
    public:
        static void Init(AAssetManager* nativeAssetManager);

        static bool ReadTextFile(const std::string& path, std::string& outText);
        static bool ReadBinaryFile(const std::string& path, std::vector<uint8_t>& outData);

    private:
        AssetManager() = default;
    };
}
