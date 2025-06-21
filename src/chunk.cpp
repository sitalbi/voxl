#include "chunk.h"
#include <iostream> 
#define FNL_IMPL
#include "FastNoiseLite.h"
#include <GLFW/glfw3.h>
#include "world.h"

Chunk::Chunk(int x, int y, int z, World* world)
	: m_indexCount(0)
{
	m_x = x * CHUNK_SIZE;
	m_y = y * CHUNK_HEIGHT;
	m_z = z * CHUNK_SIZE;
	m_world = world;
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

BlockType Chunk::getBlockType(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) {
        return BlockType::None;
    }
    return cubes[x][y][z];
}

BlockType Chunk::getBlockTypeWorldPos(int worldX, int worldY, int worldZ) const
{
    // Convert world coordinates to local chunk coordinates
    int localX = worldX - m_x;
    int localY = worldY - m_y;
    int localZ = worldZ - m_z;

	return getBlockType(localX, localY, localZ);
}

void Chunk::generate()
{
    m_indexCount = 0;
    BlockType type = BlockType::None;
    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.015f;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Generate a height for the current (x, z) position based on noise
            float noiseValue = fnlGetNoise2D(&noise, m_x + x, m_z + z);
            int maxHeight = static_cast<int>((noiseValue + 1.0f) * (CHUNK_HEIGHT / 2));

            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                type = BlockType::None;
                if (y < maxHeight) {
                    if (y < maxHeight - 4) {
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

	
}

void Chunk::generateGreedyMesh()
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> textures;
    std::vector<unsigned int> indices;

    // All 6 directions
    std::vector<glm::vec3> directions = {
        {-1, 0, 0},  // 0: left   
        { 1, 0, 0},  // 1: right  
        { 0,-1, 0},  // 2: bottom 
        { 0, 1, 0},  // 3: top    
        { 0, 0,-1},  // 4: back   
        { 0, 0, 1}   // 5: front  
    };

    // Process each direction separately
    for (const glm::vec3& dir : directions) {
        processDirection(dir, vertices, normals, textures, indices);
    }

    m_mesh = std::make_unique<Mesh>();
    m_mesh->setVertices(vertices);
    m_mesh->setNormals(normals);
    m_mesh->setTexCoords(textures);
    m_mesh->setIndices(indices);
    m_indexCount = indices.size();
    m_mesh->createMesh();
}

void Chunk::processDirection(const glm::vec3& dir,
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec3>& textures,
    std::vector<unsigned int>& indices)
{
    memset(m_visited, false, sizeof(m_visited));
    std::vector<Quad> quads;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                glm::ivec3 currentPos(x, y, z);
                BlockType blockType = cubes[x][y][z];

                if (m_visited[x][y][z] || !isBlockFaceVisible(x, y, z, dir, blockType)) {
                    continue;
                }

				m_visited[x][y][z] = true; // Mark as visited

                // Find the dimensions to expand based on direction
                auto [width, height] = expandQuad(currentPos, dir, blockType);

                quads.push_back({ glm::vec3(x, y, z), glm::vec2(width, height), dir, blockType });
            }
        }
    }

    // Generate mesh from quads for this direction
    for (const auto& quad : quads) {
        generateQuadGeometry(quad, vertices, normals, textures, indices);
    }
}

std::pair<int, int> Chunk::expandQuad(const glm::ivec3& startPos, const glm::vec3& dir,
    BlockType blockType)
{
    int width = 1, height = 1;

    // Determine which axes to expand based on direction
    glm::ivec3 widthAxis, heightAxis;
    getExpansionAxes(dir, widthAxis, heightAxis);

    // Expand width first
    while (true) {
        glm::ivec3 nextPos = startPos + widthAxis * width;

        if (!isValidPosition(nextPos) ||
            m_visited[nextPos.x][nextPos.y][nextPos.z] ||
            cubes[nextPos.x][nextPos.y][nextPos.z] != blockType ||
            !isBlockFaceVisible(nextPos.x, nextPos.y, nextPos.z, dir, blockType)) {
            break;
        }

		m_visited[nextPos.x][nextPos.y][nextPos.z] = true; // Mark as visited
        width++;
    }

    // Expand height
    for (int h = 1; h < getMaxHeight(startPos, heightAxis); h++) {
        bool rowGood = true;

        // Check entire width at this height
        for (int w = 0; w < width; w++) {
            glm::ivec3 checkPos = startPos + widthAxis * w + heightAxis * h;

            if (!isValidPosition(checkPos) ||
                m_visited[checkPos.x][checkPos.y][checkPos.z] ||
                cubes[checkPos.x][checkPos.y][checkPos.z] != blockType ||
                !isBlockFaceVisible(checkPos.x, checkPos.y, checkPos.z, dir, blockType)) {
                rowGood = false;
                break;
            }
        }

        if (rowGood) {
            height++;
            // Mark entire row as visited
            for (int w = 0; w < width; w++) {
                glm::ivec3 markPos = startPos + widthAxis * w + heightAxis * h;
				m_visited[markPos.x][markPos.y][markPos.z] = true; // Mark as visited
            }
        }
        else {
            break;
        }
    }

    return { width, height };
}

void Chunk::getExpansionAxes(const glm::vec3& dir, glm::ivec3& widthAxis, glm::ivec3& heightAxis)
{
    if (abs(dir.x) > 0) {
        // X face: expand in Y and Z
        widthAxis = glm::ivec3(0, 0, 1);  
        heightAxis = glm::ivec3(0, 1, 0); 
    }
    else if (abs(dir.y) > 0) {
        // Y face: expand in X and Z
        widthAxis = glm::ivec3(1, 0, 0);  
        heightAxis = glm::ivec3(0, 0, 1); 
    }
    else {
        // Z face: expand in X and Y
        widthAxis = glm::ivec3(1, 0, 0);  
        heightAxis = glm::ivec3(0, 1, 0); 
    }
}

bool Chunk::isValidPosition(const glm::ivec3& pos)
{
    return pos.x >= 0 && pos.x < CHUNK_SIZE &&
        pos.y >= 0 && pos.y < CHUNK_HEIGHT &&
        pos.z >= 0 && pos.z < CHUNK_SIZE;
}

int Chunk::getMaxHeight(const glm::ivec3& startPos, const glm::ivec3& heightAxis)
{
    if (heightAxis.x != 0) return CHUNK_SIZE - startPos.x;
    if (heightAxis.y != 0) return CHUNK_HEIGHT - startPos.y;
    if (heightAxis.z != 0) return CHUNK_SIZE - startPos.z;
    return 0;
}

void Chunk::generateQuadGeometry(const Quad& quad, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& textures,
    std::vector<unsigned int>& indices)
{
    glm::vec3 pos = quad.position;
    float width = quad.size.x;
    float height = quad.size.y;
    glm::vec3 dir = quad.direction;

    // Convert direction to face index
	int faceIndex = Atlas::faceIndexForDir(dir);

    glm::vec3 v1, v2, v3, v4;
    float x = pos.x, y = pos.y, z = pos.z;

    switch (faceIndex) {
    case 0: // Left face
        v1 = glm::vec3(x, y, z + width);
        v2 = glm::vec3(x, y + height, z + width);
        v3 = glm::vec3(x, y + height, z);
        v4 = glm::vec3(x, y, z);
        break;
    case 1: // Right face
        v1 = glm::vec3(x + 1, y, z);
        v2 = glm::vec3(x + 1, y + height, z);
        v3 = glm::vec3(x + 1, y + height, z + width);
        v4 = glm::vec3(x + 1, y, z + width);
        break;
    case 2: // Bottom face
        v1 = glm::vec3(x + width, y, z);
        v2 = glm::vec3(x + width, y, z + height);
        v3 = glm::vec3(x, y, z + height);
        v4 = glm::vec3(x, y, z);
        break;
    case 3: // Top face
        v1 = glm::vec3(x, y + 1, z + height);
        v2 = glm::vec3(x + width, y + 1, z + height);
        v3 = glm::vec3(x + width, y + 1, z);
        v4 = glm::vec3(x, y + 1, z);
        break;
    case 4: // Back face
        v1 = glm::vec3(x + width, y, z);
        v2 = glm::vec3(x, y, z);
        v3 = glm::vec3(x, y + height, z);
        v4 = glm::vec3(x + width, y + height, z);
        break;
    case 5: // Front face
        v1 = glm::vec3(x, y, z + 1);
        v2 = glm::vec3(x + width, y, z + 1);
        v3 = glm::vec3(x + width, y + height, z + 1);
        v4 = glm::vec3(x, y + height, z + 1);
        break;
    }

    // Add vertices
    size_t startIndex = vertices.size();
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);

    // Add normals
    for (int i = 0; i < 4; ++i) {
        normals.push_back(dir);
    }

    // Add texture 
    int layer = Atlas::getLayer(quad.type, quad.direction);
    float w = float(quad.size.x);
    float h = float(quad.size.y);

    glm::vec3 uv0, uv1, uv2, uv3;

    switch (faceIndex) {
    case 0: // Left face
        uv0 = glm::vec3(w, 0.0f, layer);
        uv1 = glm::vec3(w, h, layer);
        uv2 = glm::vec3(0.0f, h, layer);
        uv3 = glm::vec3(0.0f, 0.0f, layer);
        break;
    case 1: // Right face
        uv0 = glm::vec3(0.0f, 0.0f, layer);
        uv1 = glm::vec3(0.0f, h, layer);
        uv2 = glm::vec3(w, h, layer);
        uv3 = glm::vec3(w, 0.0f, layer);
        break;
    case 2: // Bottom face
        uv0 = glm::vec3(w, 0.0f, layer);
        uv1 = glm::vec3(w, h, layer);
        uv2 = glm::vec3(0.0f, h, layer);
        uv3 = glm::vec3(0.0f, 0.0f, layer);
        break;
    case 3: // Top face 
        uv0 = glm::vec3(0.0f, h, layer);
        uv1 = glm::vec3(w, h, layer);  
        uv2 = glm::vec3(w, 0.0f, layer); 
        uv3 = glm::vec3(0.0f, 0.0f, layer); 
        break;
    case 4: // Back face
        uv0 = glm::vec3(w, 0.0f, layer);  
        uv1 = glm::vec3(0.0f, 0.0f, layer); 
        uv2 = glm::vec3(0.0f, h, layer);  
        uv3 = glm::vec3(w, h, layer);  
        break;
    case 5: // Front face 
        uv0 = glm::vec3(0.0f, 0.0f, layer); 
        uv1 = glm::vec3(w, 0.0f, layer); 
        uv2 = glm::vec3(w, h, layer); 
        uv3 = glm::vec3(0.0f, h, layer);  
        break;
    }

    textures.push_back(uv0);
    textures.push_back(uv1);
    textures.push_back(uv2);
    textures.push_back(uv3);


    indices.push_back(startIndex);
    indices.push_back(startIndex + 1);
    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 3);
    indices.push_back(startIndex);
}


inline bool Chunk::isBlockFaceVisible(int x, int y, int z, const glm::vec3& dir, BlockType faceType) const
{
    if (faceType == BlockType::None) return false;

    int nx = x + static_cast<int>(dir.x);
    int ny = y + static_cast<int>(dir.y);
    int nz = z + static_cast<int>(dir.z);

    // if neighbor is out of chunk
    if (nx < 0 || nx >= CHUNK_SIZE ||
        ny < 0 || ny >= CHUNK_HEIGHT ||
        nz < 0 || nz >= CHUNK_SIZE)
    {
        // Neighbor is outside this chunk - check adjacent chunk
        const Chunk* neighbor = nullptr;
        int neighborX = x, neighborY = y, neighborZ = z;

        if (nx < 0) {
            // Left boundary
            neighbor = m_world->getChunk(m_x - CHUNK_SIZE, m_y, m_z);
            neighborX = CHUNK_SIZE - 1;
        }
        else if (nx >= CHUNK_SIZE) {
            // Right boundary
            neighbor = m_world->getChunk(m_x + CHUNK_SIZE, m_y, m_z);
            neighborX = 0;
        }
        else if (ny < 0) {
            // Bottom boundary
            neighbor = m_world->getChunk(m_x, m_y - CHUNK_HEIGHT, m_z);
            neighborY = CHUNK_HEIGHT - 1;
        }
        else if (ny >= CHUNK_HEIGHT) {
            // Top boundary
            neighbor = m_world->getChunk(m_x, m_y + CHUNK_HEIGHT, m_z);
            neighborY = 0;
        }
        else if (nz < 0) {
            // Back boundary
            neighbor = m_world->getChunk(m_x, m_y, m_z - CHUNK_SIZE);
            neighborZ = CHUNK_SIZE - 1;
        }
        else if (nz >= CHUNK_SIZE) {
            // Front boundary
            neighbor = m_world->getChunk(m_x, m_y, m_z + CHUNK_SIZE);
            neighborZ = 0;
        }

        if (neighbor) {
            BlockType neighborBlockType = neighbor->cubes[neighborX][neighborY][neighborZ];
            return (neighborBlockType == BlockType::None);
        }
        else {
			return true;
        }
    }

	// if neighbor is in the chunk, check if it is empty
    return cubes[nx][ny][nz] == BlockType::None;
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

