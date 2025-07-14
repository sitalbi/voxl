#include "mesh.h"
#include <glad/glad.h>
#include <iostream>

Mesh::Mesh() : VBO(0), VAO(0), EBO(0), NBO(0), TBO(0), AOBO(0), m_isSetup(false)
{
	
}

Mesh::Mesh(Mesh* other)
{
	vertices = other->vertices;
	normals = other->normals;
	texCoords = other->texCoords;
	ao = other->ao;
	indices = other->indices;
	VAO = other->VAO;
	VBO = other->VBO;
	EBO = other->EBO;
	NBO = other->NBO;
	TBO = other->TBO;
	AOBO = other->AOBO;
	m_isSetup = other->m_isSetup;
	
}

Mesh::~Mesh()
{
	if (VAO) {
		glDeleteVertexArrays(1, &VAO);
	}
	if (VBO) {
		glDeleteBuffers(1, &VBO);
	}
	if (EBO) {
		glDeleteBuffers(1, &EBO);
	}
	if (NBO) {
		glDeleteBuffers(1, &NBO);
	}
	if (TBO) {
		glDeleteBuffers(1, &TBO);
	}
	if (AOBO) {
		glDeleteBuffers(1, &AOBO);
	}
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

	indices = cube_index_data;

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
	m_isSetup = err == GL_NO_ERROR;

    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
	// Vertex array
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Vertex buffer
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	// Set vertex attribute for position (location 0)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Normal buffer
	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

	// Set vertex attribute for normals (location 1)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Texture buffer 
	glGenBuffers(1, &TBO);
	glBindBuffer(GL_ARRAY_BUFFER, TBO);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec3), texCoords.data(), GL_STATIC_DRAW);

	// Set vertex attribute for texture coordinates (location 2)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (ao.size() == vertices.size()) {
		// AO buffer
		glGenBuffers(1, &AOBO);
		glBindBuffer(GL_ARRAY_BUFFER, AOBO);
		glBufferData(GL_ARRAY_BUFFER, ao.size() * sizeof(float), ao.data(), GL_STATIC_DRAW);

		// Set vertex attribute for ambient occlusion (location 3)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	// Index buffer
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
	m_isSetup = err == GL_NO_ERROR;
}

void Mesh::draw() const
{
	if (m_isSetup) {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
	else {
		std::cerr << "Mesh is not set up!" << std::endl;
	}
}
