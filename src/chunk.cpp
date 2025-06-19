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
	generateGreedyMesh();
}

void Chunk::generateMesh()
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textures;
	std::vector<unsigned int> indices;

    std::vector<glm::vec3> directions = {
        {-1,0,0} ,{1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1}
    };

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				BlockType blockType = cubes[x][y][z];
				for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
                    if (isBlockFaceVisible(x, y, z, directions[faceIndex], blockType)) {
                        addFace(vertices, normals, textures,indices, x, y, z, faceIndex, blockType);
						for (int i = 0; i < 3; ++i) { 
                            normals.push_back(directions[faceIndex]);
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

void Chunk::generateGreedyMesh()
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textures;
    std::vector<unsigned int> indices;

    // All 6 directions: left, right, bottom, top, back, front
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
    std::vector<glm::vec2>& textures,
    std::vector<unsigned int>& indices)
{
    std::unordered_set<glm::ivec3> visited;
    std::vector<Quad> quads;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                glm::ivec3 currentPos(x, y, z);
                BlockType blockType = cubes[x][y][z];

                if (visited.contains(currentPos) || !isBlockFaceVisible(x, y, z, dir, blockType)) {
                    continue;
                }

                visited.insert(currentPos);

                // Find the dimensions to expand based on direction
                auto [width, height] = expandQuad(currentPos, dir, blockType, visited);

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
    BlockType blockType, std::unordered_set<glm::ivec3>& visited)
{
    int width = 1, height = 1;

    // Determine which axes to expand based on direction
    glm::ivec3 widthAxis, heightAxis;
    getExpansionAxes(dir, widthAxis, heightAxis);

    // Expand width first
    while (true) {
        glm::ivec3 nextPos = startPos + widthAxis * width;

        if (!isValidPosition(nextPos) ||
            visited.contains(nextPos) ||
            cubes[nextPos.x][nextPos.y][nextPos.z] != blockType ||
            !isBlockFaceVisible(nextPos.x, nextPos.y, nextPos.z, dir, blockType)) {
            break;
        }

        visited.insert(nextPos);
        width++;
    }

    // Expand height
    for (int h = 1; h < getMaxHeight(startPos, heightAxis); h++) {
        bool rowGood = true;

        // Check entire width at this height
        for (int w = 0; w < width; w++) {
            glm::ivec3 checkPos = startPos + widthAxis * w + heightAxis * h;

            if (!isValidPosition(checkPos) ||
                visited.contains(checkPos) ||
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
                visited.insert(markPos);
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

void Chunk::generateQuadGeometry(const Quad& quad,
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& textures,
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

    // Add texture coordinates
    const AtlasTile& tile = Atlas::getTile(quad.type, dir);
    glm::vec2 uv1, uv2, uv3, uv4;

    switch (faceIndex) {
    case 0: // Left face
        uv1 = tile.uv00;
        uv2 = tile.uv01;
        uv3 = tile.uv11;
        uv4 = tile.uv10;
        break;
    case 1: // Right face
        uv1 = tile.uv00;
        uv2 = tile.uv01;
        uv3 = tile.uv11;
        uv4 = tile.uv10;
        break;
    case 2: // Bottom face
        uv1 = tile.uv10;
        uv2 = tile.uv00;
        uv3 = tile.uv01;
        uv4 = tile.uv11;
        break;
    case 3: // Top face
        uv1 = tile.uv00;
        uv2 = tile.uv10;
        uv3 = tile.uv11;
        uv4 = tile.uv01;
        break;
    case 4: // Back face
        uv1 = tile.uv00;
        uv2 = tile.uv10;
        uv3 = tile.uv11;
        uv4 = tile.uv01;
        break;
    case 5: // Front face
        uv1 = tile.uv10;
        uv2 = tile.uv00;
        uv3 = tile.uv01;
        uv4 = tile.uv11;
        break;
    }

    textures.push_back(uv1);
    textures.push_back(uv2);
    textures.push_back(uv3);
    textures.push_back(uv4);

    indices.push_back(startIndex);
    indices.push_back(startIndex + 1);
    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 3);
    indices.push_back(startIndex);
}


bool Chunk::isBlockFaceVisible(int x, int y, int z, const glm::vec3& dir, BlockType faceType)
{
    if (faceType == BlockType::None) return false;

    int nx = x + static_cast<int>(dir.x);
    int ny = y + static_cast<int>(dir.y);
    int nz = z + static_cast<int>(dir.z);

    // if neighbor is out of chunk → face is visible
    if (nx < 0 || nx >= CHUNK_SIZE ||
        ny < 0 || ny >= CHUNK_HEIGHT ||
        nz < 0 || nz >= CHUNK_SIZE)
    {
        return true;
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

void Chunk::addFace(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& textures, std::vector<uint32_t>& indices,
    int x, int y, int z, int faceIndex, BlockType type) {
    glm::vec3 v1, v2, v3, v4;
	glm::vec2 uv1, uv2, uv3, uv4;

	AtlasTile tile = Atlas::blockUVs[static_cast<size_t>(type)][faceIndex];

    switch (faceIndex) {
    case 0: // Left face
        v1 = glm::vec3(x, y, z + 1);
        v2 = glm::vec3(x, y + 1, z + 1);
        v3 = glm::vec3(x, y + 1, z);
        v4 = glm::vec3(x, y, z);
		
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

