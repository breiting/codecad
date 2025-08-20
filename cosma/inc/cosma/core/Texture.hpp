#pragma once

#include <string>

class Texture {
   public:
    Texture(const std::string& path);
    ~Texture();

    void Bind(unsigned int texLoc) const;
    void Unbind() const;

    int GetWidth() const {
        return m_Width;
    }
    int GetHeight() const {
        return m_Height;
    }
    float GetAspectRatio() const {
        return static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }

   private:
    unsigned int m_TextureId;
    int m_Width, m_Height, m_Channels;
};
