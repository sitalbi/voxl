#pragma once


class Texture {

public:
	Texture();
	~Texture();

	bool loadFromFile(const char* filePath);
	void bind(unsigned int slot = 0) const;
	void unbind() const;
	unsigned int getID() const { return m_textureID; }

private:

	unsigned int m_textureID;
	int width, height;
};