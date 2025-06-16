#pragma once

#include "cube.h"
#include "mesh.h"
#include <vector>

class Chunk {


public:
	static const int CHUNK_SIZE = 32;
	static const int CHUNK_HEIGHT = 32;

	Chunk(int x = 0, int y = 0, int z = 0);
	Chunk(const Chunk* chunk);
	~Chunk();

	BlockType cubes[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	Mesh* getMesh() { return m_mesh.get(); }
	Mesh* getWaterMesh() { return m_waterMesh.get(); }

	glm::vec3 getPosition() const { return glm::vec3(m_x, m_y, m_z); }
	int getIndexCount() { return m_indexCount; }

	void setBlockType(int x, int y, int z, BlockType type);

	void generate();
	void generateMesh();

	bool isBlockFaceVisible(int x, int y, int z, int direction, BlockType faceType);

	void draw() const;

private:
	int m_x, m_y, m_z;
	int m_indexCount;


	std::unique_ptr<Mesh> m_mesh;
	std::unique_ptr<Mesh> m_waterMesh;

	void addFace(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& textures, std::vector<unsigned int>& indices,
		int x, int y, int z, int faceIndex, BlockType type);
};