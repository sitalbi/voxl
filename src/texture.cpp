#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "texture.h"
#include <glad/glad.h>
#include <iostream>

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

void Texture::bind(unsigned int slot) const
{
	if (slot >= 0 && slot < 32) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
	}
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
