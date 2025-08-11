#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct TextureArray {
	static const int getLayerIndex(int type, const glm::vec3& dir) {

	}
};

class Texture {

public:
	Texture();
	~Texture();

	bool loadFromFile(const char* filePath);
	bool loadTextureArrayFromFile(const char* filePath, int cols, int rows);
	bool loadCubemap(const char* filePath);
	void bind(unsigned int slot = 0) const;
	void bindCubemap(unsigned int slot = 0) const;
	void unbind() const;
	unsigned int getID() const { return m_textureID; }

private:

	unsigned int m_textureID;
};