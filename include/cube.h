#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "mesh.h"
#include <array>

enum class BlockType {
	None = 0,
	Grass,
	Dirt,
	Stone,
	Sand,
	Tree,
	Wood,
	Water,
	Snow,
	Leaves,
	NUM
};

struct AtlasTile {
	glm::vec2 uv00, uv10, uv11, uv01;
	int   col, row;
};

// Struct defining the texture atlas
struct Atlas {
	static constexpr int COLS = 16;
	static constexpr int ROWS = 16;
	static constexpr size_t TYPE_COUNT = size_t(BlockType::NUM);
	static constexpr size_t FACE_COUNT = 6;

	static constexpr AtlasTile makeRect(int col, int row) {
		float du = 1.0f / float(COLS), dv = 1.0f / float(ROWS);
		float u0 = col * du, v0 = row * dv;
		return AtlasTile{
		  {u0,      v0    },
		  {u0 + du,   v0    },
		  {u0 + du,   v0 + dv },
		  {u0,      v0 + dv },
		  col, row
		};
	}

	static inline int getLayer(BlockType type, const glm::vec3& dir) {
		auto& t = getTile(type, dir);
		return t.row * COLS + t.col;
	}

	static inline int faceIndexForDir(const glm::vec3& dir) {
		if (dir.x < 0) return 0;  // left
		else if (dir.x > 0) return 1;  // right
		else if (dir.y < 0) return 2;  // bottom
		else if (dir.y > 0) return 3;  // top
		else if (dir.z < 0) return 4;  // back
		else /*dir.z>0*/   return 5;  // front
	}

	static inline const AtlasTile& getTile(BlockType type, const glm::vec3& dir) {
		int idx = faceIndexForDir(dir);
		return blockUVs[size_t(type)][idx];
	}

	static inline const std::array<std::array<AtlasTile, FACE_COUNT>, TYPE_COUNT> blockUVs = {{
		// Rows, Columns
		
		// BlockType::None  (0)
		{{
			makeRect(0,1), makeRect(0,1),
			makeRect(0,1), makeRect(0,1),
			makeRect(0,1), makeRect(0,1)
		}},
		// BlockType::Grass (1)
		{{
			makeRect(3,15), // left
			makeRect(3,15), // right
			makeRect(2,15), // bottom
			makeRect(0,15), // top
			makeRect(3,15), // back
			makeRect(3,15)  // front
		}},
		// BlockType::Dirt  (2)
		{{
			makeRect(2,15), // left
			makeRect(2,15), // right
			makeRect(2,15), // bottom
			makeRect(2,15), // top
			makeRect(2,15), // back
			makeRect(2,15)  // front
		}},
		// BlockType::Stone (3)
		{{
			makeRect(1,15), // left
			makeRect(1,15), // right
			makeRect(1,15), // bottom
			makeRect(1,15), // top
			makeRect(1,15), // back
			makeRect(1,15)  // front
		}},
		// BlockType::Sand  (4)
		{{
			makeRect(3,14), // left
			makeRect(3,14), // right
			makeRect(3,14), // bottom
			makeRect(3,14), // top
			makeRect(3,14), // back
			makeRect(3,14)  // front
		}},
		// BlockType::Tree  (5)
		{{
			makeRect(4,14), // left
			makeRect(4,14), // right
			makeRect(5,14), // bottom
			makeRect(5,14), // top
			makeRect(4,14), // back
			makeRect(4,14)  // front
		}},
		// BlockType::Wood  (6)
		{{
			makeRect(4,15), // left
			makeRect(4,15), // right
			makeRect(4,15), // bottom
			makeRect(4,15), // top
			makeRect(4,15), // back
			makeRect(4,15)  // front
		}},
		// BlockType::Water (7)
		{{
			makeRect(15,3), // left
			makeRect(15,3), // right
			makeRect(15,3), // bottom
			makeRect(15,3), // top
			makeRect(15,3), // back
			makeRect(15,3)  // front
		}},
		// BlockType::Snow  (8)
		{{
			makeRect(2,11), // left
			makeRect(2,11), // right
			makeRect(2,11), // bottom
			makeRect(2,11), // top
			makeRect(2,11), // back
			makeRect(2,11)  // front
		}},
		// BlockType::Leaves  (9)
		{{
			makeRect(4,12), // left
			makeRect(4,12), // right
			makeRect(4,12), // bottom
			makeRect(4,12), // top
			makeRect(4,12), // back
			makeRect(4,12)  // front
		}}
	}};

};

class Cube {
public:
	Cube(BlockType type, glm::vec3 position);
	~Cube();

	BlockType getType() const { return m_type; }
	glm::vec3 getWorldPosition() const { return m_position; }
	glm::mat4 getModelMatrix() const { return m_modelMatrix; }

	void draw() const;

private:
	std::unique_ptr<Mesh> m_mesh;

	BlockType m_type;

	glm::vec3 m_position;

	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
};