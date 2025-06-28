#pragma once

#include <vector>
#include <array>
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

	void setupMesh();

    void draw() const;

	void setVertices(const std::vector<glm::vec3>& vert) {
		vertices = vert;
	}

	void setNormals(const std::vector<glm::vec3>& norm) {
		normals = norm;
	}

	void setTexCoords(const std::vector<glm::vec3>& tex) {
		texCoords = tex;
	}

	void setIndices(const std::vector<unsigned int>& ind) {
		indices = ind;
	}

	bool m_isSetup = false;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> colors;
    std::vector<glm::vec3> texCoords;
    std::vector<float> ao;

	std::vector<unsigned int> indices;

private:
	unsigned int VAO, VBO, EBO, NBO, TBO, AOBO;


};

enum class BlockType;
struct Quad {
	glm::vec3 position; // Position of the quad
	glm::vec2 size; // Size of the quad (width, height)
    glm::vec3 direction; // Direction of the quad (0-5 for each face)
	BlockType type; // Type of block (for texture mapping)
    std::array<float, 4> ao; // Ambient occlusion values for the 4 corners of the quad
};;