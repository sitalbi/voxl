#include "cube.h"
#include <iostream>

Cube::Cube(BlockType type, glm::vec3 position) : m_type(type), m_position(position)
{
	// Create the mesh for the cube
	m_mesh = std::make_unique<Mesh>();
	m_mesh->createCube();
}

Cube::~Cube()
{
}

void Cube::draw() const
{
	if (m_mesh)
	{
		m_mesh->draw();
	}
	else
	{
		std::cerr << "Cube mesh is not initialized!" << std::endl;
	}
}
