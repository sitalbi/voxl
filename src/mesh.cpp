#include "mesh.h"
#include <glad/glad.h>
#include <iostream>

Mesh::Mesh() : VBO(0), VAO(0), EBO(0)
{
}

Mesh::~Mesh()
{
}

void Mesh::createCube()
{

    // Vertex array
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cube_vertex_data.size() * sizeof(glm::vec3), cube_vertex_data.data(), GL_STATIC_DRAW);

    // Set vertex attribute for position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Index buffer
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_index_data.size() * sizeof(unsigned int), cube_index_data.data(), GL_STATIC_DRAW);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }

	m_indices = cube_index_data;

    glBindVertexArray(0);
}

void Mesh::createMesh()
{
	// Vertex array
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Vertex buffer
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), m_vertices.data(), GL_STATIC_DRAW);

	// Set vertex attribute for position (location 0)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Normal buffer
	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), m_normals.data(), GL_STATIC_DRAW);

	// Set vertex attribute for normals (location 1)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Texture buffer 
	glGenBuffers(1, &TBO);
	glBindBuffer(GL_ARRAY_BUFFER, TBO);
	glBufferData(GL_ARRAY_BUFFER, m_texCoords.size() * sizeof(glm::vec2), m_texCoords.data(), GL_STATIC_DRAW);

	// Set vertex attribute for texture coordinates (location 2)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Index buffer
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
}

void Mesh::draw() const
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
