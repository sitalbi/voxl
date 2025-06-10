#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "mesh.h"

enum class BlockType {
	None = 0,
	Grass,
	Dirt,
	Stone,
	Sand,
	Wood,
	Water,
	Leaves,
	Snow
};

class Cube {
public:
	Cube(BlockType type, glm::vec3 position);
	~Cube();

	BlockType getType() const { return m_type; }
	glm::vec3 getPosition() const { return m_position; }
	glm::mat4 getModelMatrix() const { return m_modelMatrix; }

	void draw() const;

private:
	std::unique_ptr<Mesh> m_mesh;

	BlockType m_type;

	glm::vec3 m_position;

	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
};