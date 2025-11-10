#include "AssetManager.h"

#include <android/asset_manager.h>

namespace Ray
{
    namespace
    {
        AAssetManager* s_NativeAssetManager = nullptr;

        bool S_NativeLoadTextFile(std::string_view fileName, std::string& outText)
        {
            RAY_CORE_ASSERT(s_NativeAssetManager != nullptr, "[AssetManager] Native Asset Manager is null!");
            // If the file hasn't been uncompressed, load it to the internal storage.
            // Note that AAsset_openFileDescriptor doesn't support compressed files (.obj).
            AAsset* asset = AAssetManager_open(s_NativeAssetManager, fileName.data(), AASSET_MODE_STREAMING);
            if (asset == nullptr)
            {
                RAY_CORE_ERROR("Could not open asset: {}", fileName);
                return false;
            }

            off_t file_size = AAsset_getLength(asset);
            outText.resize(file_size);
            int ret = AAsset_read(asset, &outText.front(), file_size);

            if (ret <= 0) {
                RAY_CORE_ERROR("Failed to open file: {}", fileName);
                AAsset_close(asset);
                return false;
            }

            AAsset_close(asset);
            return true;
        }

        bool S_NativeLoadBinaryFile(std::string_view fileName, std::vector<uint8_t>& outData)
        {
            RAY_CORE_ASSERT(s_NativeAssetManager != nullptr, "[AssetManager] Native Asset Manager is null!");
            // If the file hasn't been uncompressed, load it to the internal storage.
            // Note that AAsset_openFileDescriptor doesn't support compressed files (.obj).
            AAsset* asset = AAssetManager_open(s_NativeAssetManager, fileName.data(), AASSET_MODE_STREAMING);
            if (asset == nullptr)
            {
                RAY_CORE_ERROR("Could not open asset: {}", fileName);
                return false;
            }

            off_t file_size = AAsset_getLength(asset);
            outData.resize(file_size);
            int ret = AAsset_read(asset, &outData.front(), file_size);

            if (ret <= 0) {
                RAY_CORE_ERROR("Failed to open file: {}", fileName);
                AAsset_close(asset);
                return false;
            }

            AAsset_close(asset);
            return true;
        }
    }

    void AssetManager::Init(AAssetManager* nativeAssetManager)
    {
        s_NativeAssetManager = nativeAssetManager;
    }

    bool AssetManager::ReadTextFile(const std::string &path, std::string &outText)
    {
        RAY_CORE_ASSERT(s_NativeAssetManager != nullptr, "Native AAssetManager is null!");
        return S_NativeLoadTextFile(path, outText);
    }

    bool AssetManager::ReadBinaryFile(const std::string &path, std::vector<uint8_t> &outData)
    {
        RAY_CORE_ASSERT(s_NativeAssetManager != nullptr, "Native AAssetManager is null!");
        return S_NativeLoadBinaryFile(path, outData);
    }
}