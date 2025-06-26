#pragma once

#include "cube.h"
#include "mesh.h"
#include <vector>
#include <unordered_set>


namespace std {
	template<> struct hash<glm::ivec3> {
		size_t operator()(glm::ivec3 const& v) const noexcept {
			// a simple but decent 3-component hash:
			return  ((uint32_t)v.x) ^
				(((uint32_t)v.y) << 11) ^
				(((uint32_t)v.z) << 22);
		}
	};
	template<> struct equal_to<glm::ivec3> {
		bool operator()(glm::ivec3 const& a, glm::ivec3 const& b) const noexcept {
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}
	};
}


class World;

class Chunk {


public:
	static const int CHUNK_SIZE = 48;
	static const int CHUNK_HEIGHT = 48;
	static const int WATER_HEIGHT = 15; 

	Chunk(int x = 0, int y = 0, int z = 0, World* world = nullptr);
	Chunk(const Chunk* chunk);
	~Chunk();

	BlockType cubes[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	Mesh* getMesh() { return m_mesh.get(); }
	Mesh* getTransparentMesh() { return m_transparentMesh.get(); }

	glm::vec3 getWorldPosition() const { return glm::vec3(m_x, m_y, m_z); }
	glm::ivec3 getPositionGrid() const { return glm::ivec3(m_x / CHUNK_SIZE, m_y / CHUNK_HEIGHT, m_z / CHUNK_SIZE); }
	int getIndexCount() { return m_indexCount; }

	void setBlockType(int x, int y, int z, BlockType type);

	BlockType getBlockType(int x, int y, int z) const;
	BlockType getBlockTypeWorldPos(int worldX, int worldY, int worldZ) const;

	// Load chunk data from noise function
	void load();

	// Generate mesh data for the chunk using greedy meshing
	void generateMeshData();


	inline bool isBlockFaceVisible(int x, int y, int z, const glm::vec3& dir, BlockType faceType) const;

	inline bool isTransparentBlock(BlockType type) const {
		return type == BlockType::Water || type == BlockType::Leaves;
	}

	void draw() const;
	void drawTransparent() const;

private:
	int m_x, m_y, m_z;
	int m_indexCount;

	bool m_visited[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	World* m_world;

	std::unique_ptr<Mesh> m_mesh;
	std::unique_ptr<Mesh> m_transparentMesh;

	void processDirection(const glm::vec3& dir);

	std::pair<int, int> expandQuad(const glm::ivec3& startPos, const glm::vec3& dir,
		BlockType blockType);

	void getExpansionAxes(const glm::vec3& dir, glm::ivec3& widthAxis, glm::ivec3& heightAxis);

	bool isValidPosition(const glm::ivec3& pos);

	int getMaxHeight(const glm::ivec3& startPos, const glm::ivec3& heightAxis);

	void generateQuadGeometry(const Quad& quad, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& textures, std::vector<unsigned int>& indices);

	int getSurfaceY(int x, int z) const;

	void plantTree(int x, int y, int z);
};