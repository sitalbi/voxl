#pragma once

#include "cube.h"
#include "mesh.h"
#include <vector>
#include <unordered_set>

namespace std {
	template<> struct hash<glm::ivec3> {
		size_t operator()(glm::ivec3 const& v) const noexcept {
			// a simple 3Å]int combination
			uint64_t x = uint32_t(v.x);
			uint64_t y = uint32_t(v.y);
			uint64_t z = uint32_t(v.z);
			return size_t(x ^ (y << 21) ^ (z << 42));
		}
	};
}

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
	void generateGreedyMesh();


	bool isBlockFaceVisible(int x, int y, int z, const glm::vec3& dir, BlockType faceType);

	void draw() const;

private:
	int m_x, m_y, m_z;
	int m_indexCount;


	std::unique_ptr<Mesh> m_mesh;
	std::unique_ptr<Mesh> m_waterMesh;

	void addFace(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& textures, std::vector<unsigned int>& indices,
		int x, int y, int z, int faceIndex, BlockType type);
	
	void processDirection(const glm::vec3& dir, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& textures, std::vector<unsigned int>& indices);


	std::pair<int, int> expandQuad(const glm::ivec3& startPos, const glm::vec3& dir,
		BlockType blockType, std::unordered_set<glm::ivec3>& visited);

	void getExpansionAxes(const glm::vec3& dir, glm::ivec3& widthAxis, glm::ivec3& heightAxis);

	bool isValidPosition(const glm::ivec3& pos);

	int getMaxHeight(const glm::ivec3& startPos, const glm::ivec3& heightAxis);

	void generateQuadGeometry(const Quad& quad, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& textures, std::vector<unsigned int>& indices);

};