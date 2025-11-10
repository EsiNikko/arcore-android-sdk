#pragma once
#include "Core/Core.h"

#include <string>
#include <map>
#include <android/asset_manager.h>
#include <glm.hpp>

namespace Ray
{
    class Shader
    {
    public:
        using Switch = std::map<std::string, int>;
        Shader(std::string vertexShaderPath, std::string fragmentShaderPath, Switch staticSwitches = Switch());
        ~Shader();

        void Invalidate();
        void SetStaticSwitches(const Switch& staticSwitches, bool reloadShader = true);

        void Bind();
        void Unbind();

        uint32_t GetUniformLocation(const std::string& name);
        uint32_t GetAttribLocation(const std::string& name);

        bool SetFloat(const std::string& name, float value);
        bool SetInt(const std::string& name, int value);
        bool SetVec2(const std::string& name, const glm::vec2& value);
        bool SetVec3(const std::string& name, const glm::vec3& value);
        bool SetVec4(const std::string& name, const glm::vec4& value);
        bool SetMat2(const std::string& name, const glm::mat2& value);
        bool SetMat3(const std::string& name, const glm::mat3& value);
        bool SetMat4(const std::string& name, const glm::mat4& value);

        void SetFloat(uint32_t location, float value);
        void SetInt(uint32_t location, int value);
        void SetVec2(uint32_t location, const glm::vec2& value);
        void SetVec3(uint32_t location, const glm::vec3& value);
        void SetVec4(uint32_t location, const glm::vec4& value);
        void SetMat2(uint32_t location, const glm::mat2& value);
        void SetMat3(uint32_t location, const glm::mat3& value);
        void SetMat4(uint32_t location, const glm::mat4& value);

        bool SetVertexAttribFloatPointer(const std::string& name, uint32_t size, bool normalized, uint32_t stride, const void* data);
        void SetVertexAttribFloatPointer(uint32_t location, uint32_t size, bool normalized, uint32_t stride, const void* data);

    private:
        AAssetManager* m_assetManager = nullptr;
        uint32_t m_rendererID = 0;
        std::string m_vertexShaderPath;
        std::string m_fragmentShaderPath;
        Switch m_staticSwitches;
        std::map<std::string, uint32_t> m_uniformLocations;
        std::map<std::string, uint32_t> m_attribLocations;
    };
}