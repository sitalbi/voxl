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
	stbi_set_flip_vertically_on_load(false);
    int w, h;
	unsigned char* data = stbi_load(filePath, &w, &h, nullptr, 4);
	if (!data) {
		std::cerr << "Failed to load texture: " << filePath << std::endl;
		return false;
	}
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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
    // load atlas
    int atlasW, atlasH, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* atlasData =
        stbi_load(filePath, &atlasW, &atlasH, &channels, 4);
    if (!atlasData) {
        std::cerr << "Failed to load atlas: " << filePath << "\n";
        return false;
    }

    if (atlasW % cols != 0 || atlasH % rows != 0) {
        std::cerr << "Atlas size " << atlasW << "Å~" << atlasH
            << " not divisible by " << cols << "Å~" << rows << "\n";
        stbi_image_free(atlasData);
        return false;
    }
    int tileW = atlasW / cols;
    int tileH = atlasH / rows;
    int layerCount = cols * rows;

    int mipLevels = static_cast<int>(std::floor(std::log2(std::max(tileW, tileH)))) + 1;

    // Create the textureÅ]array
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
    
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
        mipLevels,  
        GL_RGBA8,
        tileW, tileH,
        layerCount);

    // Set each atlas cell to a layer
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
                0,
                0, 0, layer,
                tileW, tileH, 1,
                GL_RGBA, GL_UNSIGNED_BYTE,
                slice.data());
        }
    }

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	// Texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(atlasData);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return true;
}

bool Texture::loadCubemap(const char* filePath)
{
    // 0) Load the atlas (disable flip so row math is predictable)
    stbi_set_flip_vertically_on_load(false);
    int atlasW = 0, atlasH = 0, channels = 0;
    unsigned char* atlas = stbi_load(filePath, &atlasW, &atlasH, &channels, 4);
    if (!atlas) { std::cerr << "Failed to load: " << filePath << "\n"; return false; }

    // 1) Grid info (adjust if your layout differs)
    int cols = 3, rows = 2;              // 3Å~2 grid of faces
    int faceW = atlasW / cols;
    int faceH = atlasH / rows;

    // 2) Create cubemap
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // 3) Tell GL how to walk rows in the big atlas
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, atlasW);

    // 4) Map grid cells -> cube faces (col,row). Tweak to match your atlas.
    struct Face { GLenum target; int c, r; };
    Face map[6] = {
        { GL_TEXTURE_CUBE_MAP_POSITIVE_X, 2, 0 }, // +X (right)
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 1 }, // -X (left)
        { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1, 0 }, // +Y (top)
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0 }, // -Y (bottom)
        { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 2, 1 }, // +Z (front)
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 1, 1 }, // -Z (back)
    };

    // 5) Upload each face by skipping into the atlas
    for (const Face& f : map) {
        int skipX = f.c * faceW;
        int skipY = f.r * faceH;
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipX);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, skipY);

        glTexImage2D(f.target, 0,
            GL_RGBA,                // or GL_RGBA8 if youÅfre not using sRGB
            faceW, faceH, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            atlas);
    }

    // 6) Reset pixel store + cleanup
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    stbi_image_free(atlas);

    return true;
}



void Texture::bind(unsigned int slot) const
{
	if (slot >= 0 && slot < 32) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
	}
}

void Texture::bindCubemap(unsigned int slot) const
{
    if (slot >= 0 && slot < 32) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
    }
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
