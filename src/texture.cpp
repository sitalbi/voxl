#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "texture.h"
#include <glad/glad.h>
#include <iostream>
#include <cassert>

Texture::Texture()
{
}

Texture::~Texture()
{
}

bool Texture::loadFromFile(const char* filePath)
{
	// Load the texture using an image loading library (e.g., stb_image)
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filePath, &width, &height, nullptr, 4);
	if (!data) {
		std::cerr << "Failed to load texture: " << filePath << std::endl;
		return false;
	}
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	stbi_image_free(data);
	return true;
}

bool Texture::loadTextureArrayFromFile(const char* filePath,
    int cols, int rows)
{
    // 1) load the entire atlas
    int atlasW, atlasH, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* atlasData =
        stbi_load(filePath, &atlasW, &atlasH, &channels, 4);
    if (!atlasData) {
        std::cerr << "Failed to load atlas: " << filePath << "\n";
        return false;
    }

    // 2) compute per-tile size
    if (atlasW % cols != 0 || atlasH % rows != 0) {
        std::cerr << "Atlas size " << atlasW << "~" << atlasH
            << " not divisible by " << cols << "~" << rows << "\n";
        stbi_image_free(atlasData);
        return false;
    }
    int tileW = atlasW / cols;
    int tileH = atlasH / rows;
    int layerCount = cols * rows;

    // 3) create the texture]array
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
    // allocate storage: 1 mip, RGBA8
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
        1,
        GL_RGBA8,
        tileW, tileH,
        layerCount);

    // 4) carve each atlas cell into its own layer
    std::vector<unsigned char> slice(tileW * tileH * 4);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int layer = row * cols + col;
            for (int y = 0; y < tileH; ++y) {
                unsigned char* src = atlasData
                    + ((row * tileH + y) * atlasW + col * tileW) * 4;
                unsigned char* dst = slice.data() + y * tileW * 4;
                memcpy(dst, src, tileW * 4);
            }
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                0,      // mip
                0, 0, layer,
                tileW, tileH, 1,
                GL_RGBA, GL_UNSIGNED_BYTE,
                slice.data());
        }
    }

    // 5) set params
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(atlasData);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return true;
}


void Texture::bind(unsigned int slot) const
{
	if (slot >= 0 && slot < 32) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
	}
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
