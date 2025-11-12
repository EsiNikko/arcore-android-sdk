#include "Shader.h"
#include "Asset/AssetManager.h"
#include "Util.h"

#include <sstream>
#include <utility>
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>

namespace Ray
{
    namespace
    {
        uint32_t S_LoadShader(GLenum shaderType, const char* shaderSource)
        {
            GLuint shader = glCreateShader(shaderType);
            if (!shader) return 0;

            glShaderSource(shader, 1, &shaderSource, nullptr);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled)
            {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen)
                {
                    std::string info;
                    info.resize(infoLen);
                    glGetShaderInfoLog(shader, infoLen, nullptr, (GLchar*)info.data());
                    RAY_CORE_ERROR("[Ray::Shader] Could not compile shader: {}", info.c_str());
                }
                glDeleteShader(shader);
                return 0;
            }
            return shader;
        }

        uint32_t S_CreateProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const Shader::Switch& macros)
        {
            std::string vertexShaderContent;
            if (!AssetManager::ReadTextFile(vertexShaderPath, vertexShaderContent)) {
                RAY_CORE_ERROR("[Ray::Shader] Failed to load file: {}", vertexShaderPath.c_str());
                return 0;
            }

            std::string fragmentShaderContent;
            if (!AssetManager::ReadTextFile(fragmentShaderPath, fragmentShaderContent)) {
                RAY_CORE_ERROR("[Ray::Shader] Failed to load file: {}", fragmentShaderPath.c_str());
                return 0;
            }

            // Prepend any #define values specified during this run.
            std::stringstream defines;
            for (const auto& entry : macros)
                defines << "#define " << entry.first << " " << entry.second << "\n";
            fragmentShaderContent = defines.str() + fragmentShaderContent;
            vertexShaderContent = defines.str() + vertexShaderContent;

            // Compiles shader code.
            GLuint vertexShader = S_LoadShader(GL_VERTEX_SHADER, vertexShaderContent.c_str());
            if (!vertexShader) return 0;

            GLuint fragmentShader = S_LoadShader(GL_FRAGMENT_SHADER, fragmentShaderContent.c_str());
            if (!fragmentShader)
            {
                glDeleteShader(vertexShader);
                return 0;
            }

            GLuint program = glCreateProgram();
            if (!program)
            {
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
                return 0;
            }

            glAttachShader(program, vertexShader);
            Util::CheckGlError("[Ray::Shader] glAttachShader");
            glAttachShader(program, fragmentShader);
            Util::CheckGlError("[Ray::Shader] glAttachShader");
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE)
            {
                GLint infoLen = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen)
                {
                    std::string info;
                    info.resize(infoLen);
                    glGetProgramInfoLog(program, infoLen, nullptr, (GLchar*)info.data());
                    RAY_CORE_ERROR("[Ray::Shader] Could not link program:");
                    RAY_CORE_ERROR("  Vertex Shader: {}", vertexShaderPath);
                    RAY_CORE_ERROR("  Fragment Shader: {}", fragmentShaderPath);
                    RAY_CORE_ERROR("  Log: {}", info.c_str());
                }

                glDeleteProgram(program);
                return 0;
            }

            return program;
        }
    }

    Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath, Switch staticSwitches) :
        m_vertexShaderPath(std::move(vertexShaderPath)),
        m_fragmentShaderPath(std::move(fragmentShaderPath)),
        m_staticSwitches(std::move(staticSwitches))
    {
        RAY_CORE_TRACE("Create Shader vs: {}, fs: {}", m_vertexShaderPath, m_fragmentShaderPath);
        Invalidate();
    }

    Shader::~Shader()
    {
        RAY_CORE_TRACE("Destroy Shader vs: {}, fs: {}", m_vertexShaderPath, m_fragmentShaderPath);
        if (m_rendererID)
            glDeleteProgram(m_rendererID);
    }

    void Shader::Invalidate()
    {
        uint32_t newProgram = S_CreateProgram(m_vertexShaderPath,
                                              m_fragmentShaderPath,
                                              m_staticSwitches);
        RAY_CORE_ASSERT(newProgram, "[Shader::Invalidate] Failed to load shader!");
        //if (!newProgram) return;

        if (m_rendererID)
            glDeleteProgram(m_rendererID);
        m_rendererID = newProgram;
    }

    void Shader::SetStaticSwitches(const Switch& staticSwitches, bool reloadShader)
    {
        m_staticSwitches = staticSwitches;
        if (reloadShader)
            Invalidate();
    }

    uint32_t Shader::GetUniformLocation(const std::string& name)
    {
        if (!m_rendererID)
        {
            RAY_CORE_ERROR("[Shader::GetUniformLocation] Invalid Renderer ID, vs: {}, fs: {}", m_vertexShaderPath, m_fragmentShaderPath);
            return 0;
        }

        if (auto it= m_uniformLocations.find(name); it != m_uniformLocations.end())
            return it->second;

        auto location = glGetUniformLocation(m_rendererID, name.c_str());
        if (location == -1)
        {
            RAY_CORE_ERROR("[Shader::GetUniformLocation] Invalid uniform name '{}', vs: {}, fs: {}", name,  m_vertexShaderPath, m_fragmentShaderPath);
            return 0;
        }
        m_uniformLocations[name] = location;
        return location;
    }

    uint32_t Shader::GetAttribLocation(const std::string& name)
    {
        if (!m_rendererID)
        {
            RAY_CORE_ERROR("[Shader::GetAttribLocation] Invalid Renderer ID, vs: {}, fs: {}", m_vertexShaderPath, m_fragmentShaderPath);
            return 0;
        }

        if (auto it= m_attribLocations.find(name); it != m_attribLocations.end())
            return it->second;

        auto location = glGetAttribLocation(m_rendererID, name.c_str());
        if (location == -1)
        {
            RAY_CORE_ERROR("[Shader::GetAttribLocation] Invalid attrib name '{}', vs: {}, fs: {}", name,  m_vertexShaderPath, m_fragmentShaderPath);
            return 0;
        }
        m_attribLocations[name] = location;
        return location;
    }

    bool Shader::SetFloat(const std::string &name, float value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetFloat(location, value);
        return true;
    }

    bool Shader::SetInt(const std::string &name, int value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetInt(location, value);
        return true;
    }

    bool Shader::SetVec2(const std::string& name, const glm::vec2& value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetVec2(location, value);
        return true;
    }
    bool Shader::SetVec3(const std::string& name, const glm::vec3& value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetVec3(location, value);
        return true;
    }
    bool Shader::SetVec4(const std::string& name, const glm::vec4& value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetVec4(location, value);
        return true;
    }
    bool Shader::SetMat2(const std::string& name, const glm::mat2& value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetMat2(location, value);
        return true;
    }
    bool Shader::SetMat3(const std::string& name, const glm::mat3& value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetMat3(location, value);
        return true;
    }
    bool Shader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        uint32_t location = GetUniformLocation(name);
        if (!location) return false;

        SetMat4(location, value);
        return true;
    }

    void Shader::SetFloat(uint32_t location, float value)
    {
        glUniform1f((GLint)location, value);
    }

    void Shader::SetInt(uint32_t location, int value)
    {
        glUniform1i((GLint)location, value);
    }

    void Shader::SetVec2(uint32_t location, const glm::vec2 &value)
    {
        glUniform2fv((GLint)location, 1, glm::value_ptr(value));
    }

    void Shader::SetVec3(uint32_t location, const glm::vec3 &value)
    {
        glUniform3fv((GLint)location, 1, glm::value_ptr(value));
    }

    void Shader::SetVec4(uint32_t location, const glm::vec4 &value)
    {
        glUniform4fv((GLint)location, 1, glm::value_ptr(value));
    }

    void Shader::SetMat2(uint32_t location, const glm::mat2 &value)
    {
        glUniformMatrix2fv((GLint)location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat3(uint32_t location, const glm::mat3 &value)
    {
        glUniformMatrix3fv((GLint)location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat4(uint32_t location, const glm::mat4 &value)
    {
        glUniformMatrix4fv((GLint)location, 1, GL_FALSE, glm::value_ptr(value));
    }

    bool Shader::SetVertexAttribFloatPointer(const std::string& name, uint32_t size, bool normalized, uint32_t stride, const void* data)
    {
        uint32_t location = GetAttribLocation(name);
        if (!location) return false;

        SetVertexAttribFloatPointer(location, size, normalized, stride, data);
        return true;
    }
    void Shader::SetVertexAttribFloatPointer(uint32_t location, uint32_t size, bool normalized, uint32_t stride, const void* data)
    {
        glEnableVertexAttribArray((GLint)location);
        glVertexAttribPointer((GLint)location, (GLint)size, GL_FLOAT,
                              normalized ? GL_TRUE : GL_FALSE,(GLint)stride,data);
    }

    void Shader::Bind()
    {
        glUseProgram(m_rendererID);
    }

    void Shader::Unbind()
    {
        glUseProgram(0);
    }
}