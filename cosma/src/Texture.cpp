// clang-format off
#include <glad.h>
// clang-format on
#include <stb_image.h>

#include <core/Texture.hpp>
#include <iostream>
#include <string>

Texture::Texture(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
    if (data) {
        printf("bind texture\n");
        glGenTextures(1, &m_TextureId);
        glBindTexture(GL_TEXTURE_2D, m_TextureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = m_Channels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    } else {
        std::cerr << "Cannot load texture from file: " << path << std::endl;
    }
}

Texture::~Texture() {
    glDeleteTextures(1, &m_TextureId);
}

void Texture::Bind(unsigned int texLoc) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glUniform1i(texLoc, 0);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
