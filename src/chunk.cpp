#include "chunk.h"
#include <iostream> 
#define FNL_IMPL
#include "FastNoiseLite.h"

Chunk::Chunk(int x, int y, int z)
	: m_x(x), m_y(y), m_z(z), m_indexCount(0)
{
}

Chunk::Chunk(const Chunk* chunk)
{
	m_x = chunk->m_x;
	m_y = chunk->m_y;
	m_z = chunk->m_z;
	//m_chunkManager = chunk->m_chunkManager;
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				cubes[x][y][z] = chunk->cubes[x][y][z];
			}
		}
	}
}

Chunk::~Chunk()
{
}

void Chunk::setBlockType(int x, int y, int z, BlockType type)
{
}

void Chunk::generate()
{
	m_indexCount = 0;

	BlockType type = BlockType::None;

    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.05f;

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Generate a height for the current (x, z) position based on noise
            float noiseValue = fnlGetNoise2D(&noise, m_x + x, m_z + z);
            int maxHeight = static_cast<int>((noiseValue + 1.0f) * (CHUNK_SIZE / 2));  // Map noise to chunk height

		    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                type = BlockType::None;
                // Only set blocks up to maxHeight
                if (y < maxHeight)  
                {
                    if (y < maxHeight - 2) {
                        type = BlockType::Stone;
                    }
                    else if (y == maxHeight - 1) {
                        type = BlockType::Grass;
                    }
                    else {
                        type = BlockType::Dirt;
                    }
                }
				cubes[x][y][z] = type;
			}
		}
	}
	generateMesh();
}

void Chunk::generateMesh()
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textures;
	std::vector<unsigned int> indices;

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				BlockType blockType = cubes[x][y][z];
				if (blockType != BlockType::None) {
					for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
                        if (isBlockFaceVisible(x, y, z, faceIndex, blockType)) {
                            addFace(vertices, normals, textures,indices, x, y, z, faceIndex, blockType);
                        }
					}
				}
			}
		}
	}
	m_mesh = std::make_unique<Mesh>();
	m_mesh->setVertices(vertices);
	m_mesh->setNormals(normals);
	m_mesh->setTexCoords(textures);
	m_mesh->setIndices(indices);
	m_indexCount = indices.size();
	m_mesh->createMesh();
}

bool Chunk::isBlockFaceVisible(int x, int y, int z, int direction, BlockType faceType)
{
    switch (direction) {
    case 0:
        return x == 0 || cubes[x - 1][y][z] == BlockType::None;
        break;
	case 1:
		return x == CHUNK_SIZE - 1 || cubes[x + 1][y][z] == BlockType::None;
		break;
	case 2:
        return y == 0 || cubes[x][y - 1][z] == BlockType::None;
		break;
	case 3:
        return y == CHUNK_SIZE - 1 || cubes[x][y + 1][z] == BlockType::None;
		break;
	case 4:
		return z == 0 || cubes[x][y][z - 1] == BlockType::None;
		break;
	case 5:
        return z == CHUNK_SIZE - 1 || cubes[x][y][z + 1] == BlockType::None;
		break;
    }
	return false;
}

void Chunk::draw() const
{
	if (m_mesh) {
		m_mesh->draw();
	}
	else {
		std::cerr << "Chunk mesh is not initialized!" << std::endl;
	}
}

void Chunk::addFace(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& textures, std::vector<uint32_t>& indices,
    int x, int y, int z, int faceIndex, BlockType type) {
    glm::vec3 v1, v2, v3, v4;
    glm::vec3 normal;
	glm::vec2 uv1, uv2, uv3, uv4;

	AtlasTile tile = Atlas::blockUVs[static_cast<size_t>(type)][faceIndex];

    switch (faceIndex) {
    case 0: // Left face
        v1 = glm::vec3(x, y, z + 1);
        v2 = glm::vec3(x, y + 1, z + 1);
        v3 = glm::vec3(x, y + 1, z);
        v4 = glm::vec3(x, y, z);
        normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		
        uv1 = tile.uv00;
        uv2 = tile.uv01;
        uv3 = tile.uv11;
        uv4 = tile.uv10;
        break;
    case 1: // Right face
        v1 = glm::vec3(x + 1, y, z);
        v2 = glm::vec3(x + 1, y + 1, z);
        v3 = glm::vec3(x + 1, y + 1, z + 1);
        v4 = glm::vec3(x + 1, y, z + 1);
        normal = glm::vec3(1.0f, 0.0f, 0.0f);
        uv1 = tile.uv00;
        uv3 = tile.uv11;
        uv2 = tile.uv01;
        uv4 = tile.uv10;
        break;
    case 2: // Bottom face
        v1 = glm::vec3(x + 1, y, z);
        v2 = glm::vec3(x + 1, y, z + 1);
        v3 = glm::vec3(x, y, z + 1);
        v4 = glm::vec3(x, y, z);
        normal = glm::vec3(0.0f, -1.0f, 0.0f);
        uv1 = tile.uv10;
        uv2 = tile.uv00;
        uv3 = tile.uv01;
        uv4 = tile.uv11;
        break;
    case 3: // Top face
        v1 = glm::vec3(x, y + 1, z + 1);
        v2 = glm::vec3(x + 1, y + 1, z + 1);
        v3 = glm::vec3(x + 1, y + 1, z);
        v4 = glm::vec3(x, y + 1, z);
        normal = glm::vec3(0.0f, 1.0f, 0.0f);
        uv1 = tile.uv00;
        uv2 = tile.uv10;
        uv3 = tile.uv11;
        uv4 = tile.uv01;
        break;
    case 4: // Back face
        v1 = glm::vec3(x + 1, y, z);
        v2 = glm::vec3(x, y, z);
        v3 = glm::vec3(x, y + 1, z);
        v4 = glm::vec3(x + 1, y + 1, z);
        normal = glm::vec3(0.0f, 0.0f, -1.0f);
        uv1 = tile.uv00;
        uv2 = tile.uv10;
        uv3 = tile.uv11;
        uv4 = tile.uv01;
        break;
    case 5: // Front face
        v1 = glm::vec3(x, y, z + 1);
        v2 = glm::vec3(x + 1, y, z + 1);
        v3 = glm::vec3(x + 1, y + 1, z + 1);
        v4 = glm::vec3(x, y + 1, z + 1);
        normal = glm::vec3(0.0f, 0.0f, 1.0f); 
        uv1 = tile.uv10;
        uv2 = tile.uv00;
        uv3 = tile.uv01;
        uv4 = tile.uv11;
        break;
    default:
        return;
    }

    uint32_t baseIndex = static_cast<uint32_t>(vertices.size());
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);

    normals.push_back(normal);
    normals.push_back(normal);
    normals.push_back(normal);
    normals.push_back(normal);

    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
    indices.push_back(baseIndex);


	textures.push_back(uv1);
	textures.push_back(uv2);
	textures.push_back(uv3);
	textures.push_back(uv4);
}

