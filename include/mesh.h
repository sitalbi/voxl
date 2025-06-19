#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const std::vector<glm::vec3> cube_vertex_data = {
    // Front face
    { -0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f },

    // Back face
    { -0.5f, -0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f },
    {  0.5f, -0.5f, -0.5f },

    // Right face
    {  0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f,  0.5f },

    // Left face
    { -0.5f, -0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f },
    { -0.5f,  0.5f, -0.5f },
    { -0.5f, -0.5f, -0.5f },

    // Top face
    { -0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f },

    // Bottom face
    { -0.5f, -0.5f,  0.5f },
    { -0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f,  0.5f }
};



static std::vector<unsigned int> cube_index_data = {
    0, 1, 2, 0, 2, 3,       // Front face
    4, 5, 6, 4, 6, 7,       // Back face
    8, 9, 10, 8, 10, 11,    // Right face
    12, 13, 14, 12, 14, 15, // Left face
    16, 17, 18, 16, 18, 19, // Top face
    20, 21, 22, 20, 22, 23  // Bottom face
};

class Mesh {

public:
	Mesh();
    ~Mesh();

	void createCube();

	void createMesh();

    void draw() const;

	void setVertices(const std::vector<glm::vec3>& vertices) {
		m_vertices = vertices;
	}

	void setNormals(const std::vector<glm::vec3>& normals) {
		m_normals = normals;
	}

	void setTexCoords(const std::vector<glm::vec2>& texCoords) {
		m_texCoords = texCoords;
	}

	void setIndices(const std::vector<unsigned int>& indices) {
		m_indices = indices;
	}

private:
	unsigned int VAO, VBO, EBO, NBO, TBO;

	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec4> m_colors;
    std::vector<glm::vec2> m_texCoords;

	std::vector<unsigned int> m_indices;


};

enum class BlockType;
struct Quad {
	glm::vec3 position; // Position of the quad
	glm::vec2 size;     // Size of the quad (width, height)
    glm::vec3 direction;      // Direction of the quad (0-5 for each face)
	BlockType type;    // Type of block (for texture mapping)
};;