#include "chunk.h"
#include <iostream> 
#define FNL_IMPL
#include "FastNoiseLite.h"
#include "glad/glad.h" 
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
	m_world = chunk->m_world;
	m_mesh = std::make_unique<Mesh>(*chunk->m_mesh);
}

Chunk::~Chunk()
{
	memset(m_visited, false, sizeof(m_visited));
}

void Chunk::setBlockType(int x, int y, int z, BlockType type)
{
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) {
		return;
	}
	cubes[x][y][z] = type;
}

BlockType Chunk::getBlockType(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) {
        return BlockType::None;
    }
    return cubes[x][y][z];
}

BlockType Chunk::getBlockType(glm::ivec3 pos) const
{
	if (pos.x < 0 || pos.x >= CHUNK_SIZE || pos.y < 0 || pos.y >= CHUNK_HEIGHT || pos.z < 0 || pos.z >= CHUNK_SIZE) {
		return BlockType::None;
	}
	return cubes[pos.x][pos.y][pos.z];
}

BlockType Chunk::getBlockTypeWorldPos(int worldX, int worldY, int worldZ) const
{
    // Convert world coordinates to local chunk coordinates
    int localX = worldX - m_x;
    int localY = worldY - m_y;
    int localZ = worldZ - m_z;

	return getBlockType(localX, localY, localZ);
}

BlockType Chunk::getBlockTypeWorldPos(glm::ivec3 worldPos) const
{
    // Convert world coordinates to local chunk coordinates
    int localX = worldPos.x - m_x;
    int localY = worldPos.y - m_y;
    int localZ = worldPos.z - m_z;

    return getBlockType(localX, localY, localZ);
}

void Chunk::load()
{
    m_indexCount = 0;
    BlockType type = BlockType::None;
    fnl_state& noise = m_world->noise;
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

                // Add water blocks if maxHeight is below WATER_HEIGHT
                if (maxHeight < WATER_HEIGHT) {
                    for (int y = maxHeight; y < WATER_HEIGHT; y++) {
                        if (cubes[x][y][z] == BlockType::None) {
                            cubes[x][y][z] = BlockType::Water;
                        }
                    }
                }
            }
        }
    }


	fnl_state treeNoise = fnlCreateState();
	treeNoise.noise_type = FNL_NOISE_OPENSIMPLEX2S; 
    treeNoise.frequency = 0.5f;    
    for (int x = 2; x < CHUNK_SIZE - 2; ++x) {
        for (int z = 2; z < CHUNK_SIZE - 2; ++z) {
            float tn = fnlGetNoise2D(&treeNoise, m_x + x, m_z + z);
            if (tn > 0.8f && cubes[x][getSurfaceY(x, z)][z] == BlockType::Grass) {
                int baseY = getSurfaceY(x, z) + 1;
                plantTree(x, baseY, z);
            }
        }
    }
	
}

void Chunk::generateMeshData()
{
    m_mesh = std::make_unique<Mesh>();
    m_transparentMesh = std::make_unique<Mesh>();
    // All 6 directions
    std::vector<glm::ivec3> directions = {
        {-1, 0, 0},  // 0: left   
        { 1, 0, 0},  // 1: right  
        { 0,-1, 0},  // 2: bottom 
        { 0, 1, 0},  // 3: top    
        { 0, 0,-1},  // 4: back   
        { 0, 0, 1}   // 5: front  
    };

    // Process each direction separately
    for (const glm::ivec3& dir : directions) {
        processDirection(dir);
    }
}

void Chunk::swapMeshes()
{
	// Swap the active mesh with the generated mesh
    m_activeMesh = std::make_unique<Mesh>(m_mesh.get());
    m_activeTransparentMesh = std::make_unique<Mesh>(m_transparentMesh.get());
}

void Chunk::processDirection(const glm::ivec3& dir)
{
    // Determine which axes to expand based on direction
    glm::ivec3 widthAxis, heightAxis;
    getExpansionAxes(dir, widthAxis, heightAxis);

    memset(m_visited, false, sizeof(m_visited));

    // 1) Pre‑compute visibility & AO for every cell
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                bool vis = isBlockFaceVisible(x, y, z, dir, cubes[x][y][z]);
                m_visibilityCache[x][y][z] = vis;
                if (vis) {
                    m_aoCache[x][y][z] = getAmbientOcclusion({ x,y,z }, dir);
                }
            }
        }
    }

    std::vector<Quad> opaqueQuads;
    std::vector<Quad> transparentQuads;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                glm::ivec3 currentPos(x, y, z);
				glm::ivec3 worldPos = currentPos + glm::ivec3(m_x, m_y, m_z);
                BlockType blockType = cubes[x][y][z];

                if (m_visited[x][y][z] || !m_visibilityCache[x][y][z]) {
                    continue;
                }

                m_visited[x][y][z] = true;
				std::array<float, 4>& ao = m_aoCache[x][y][z];
                auto [width, height] = expandQuad(currentPos, dir, blockType, widthAxis, heightAxis, ao);

				Quad quad;
				quad.position = currentPos;
				quad.size = glm::vec2(width, height);
				quad.direction = dir;
				quad.type = blockType;
                quad.ao = ao;

				if (blockType == BlockType::Water) {
                    transparentQuads.push_back(quad);
                }
                else {
                    opaqueQuads.push_back(quad);
                }
                
            }
        }
    }

    // Generate mesh from quads for this direction
    for (const auto& quad : opaqueQuads) {
        generateQuadGeometry(quad, m_mesh->vertices, m_mesh->normals, m_mesh->texCoords, m_mesh->ao, m_mesh->indices);
    }
	for (const auto& quad : transparentQuads) {
		generateQuadGeometry(quad, m_transparentMesh->vertices, m_transparentMesh->normals, m_transparentMesh->texCoords, m_transparentMesh->ao, m_transparentMesh->indices);
	}
}

std::pair<int, int> Chunk::expandQuad(const glm::ivec3& startPos, const glm::vec3& dir,
	BlockType blockType, const glm::ivec3& widthAxis, const glm::ivec3& heightAxis, std::array<float, 4>& ao)
{
    int width = 1, height = 1;

    // Expand width first
    while (true) {
        glm::ivec3 nextPos = startPos + widthAxis * width;

		nextAo = m_aoCache[nextPos.x][nextPos.y][nextPos.z];
		bool visible = m_visibilityCache[nextPos.x][nextPos.y][nextPos.z];

        if (!isValidPosition(nextPos) ||
            m_visited[nextPos.x][nextPos.y][nextPos.z] ||
            cubes[nextPos.x][nextPos.y][nextPos.z] != blockType ||
            !visible ||
            ao!=nextAo) {
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

			nextAo = m_aoCache[checkPos.x][checkPos.y][checkPos.z];
			bool visible = m_visibilityCache[checkPos.x][checkPos.y][checkPos.z];

            if (!isValidPosition(checkPos) ||
                m_visited[checkPos.x][checkPos.y][checkPos.z] ||
                cubes[checkPos.x][checkPos.y][checkPos.z] != blockType ||
                !visible ||
                nextAo != ao) {
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
    std::vector<float>& ao, std::vector<unsigned int>& indices)
{
    glm::vec3 pos = quad.position;
    float width = quad.size.x;
    float height = quad.size.y;
    glm::vec3 dir = quad.direction;

    // Convert direction to face index
    int faceIndex = Atlas::faceIndexForDir(dir);

    glm::vec3 v1, v2, v3, v4;
    float x = pos.x, y = pos.y, z = pos.z;

	// Vertex positions 
    switch (faceIndex) {
    case 0: // Left face (-X)
        v1 = glm::vec3(x, y, z);             // bottom-left
        v2 = glm::vec3(x, y, z + width);     // bottom-right  
        v3 = glm::vec3(x, y + height, z);    // top-left
        v4 = glm::vec3(x, y + height, z + width); // top-right
        break;
    case 1: // Right face (+X)
        v1 = glm::vec3(x + 1, y, z + width); // bottom-left
        v2 = glm::vec3(x + 1, y, z);         // bottom-right
        v3 = glm::vec3(x + 1, y + height, z + width); // top-left
        v4 = glm::vec3(x + 1, y + height, z); // top-right
        break;
    case 2: // Bottom face (-Y)
        v1 = glm::vec3(x, y, z);             // bottom-left
        v2 = glm::vec3(x + width, y, z);     // bottom-right
        v3 = glm::vec3(x, y, z + height);    // top-left
        v4 = glm::vec3(x + width, y, z + height); // top-right
        break;
    case 3: // Top face (+Y)
        v1 = glm::vec3(x, y + 1, z + height); // bottom-left
        v2 = glm::vec3(x + width, y + 1, z + height); // bottom-right
        v3 = glm::vec3(x, y + 1, z);         // top-left
        v4 = glm::vec3(x + width, y + 1, z); // top-right
        break;
    case 4: // Back face (-Z)
        v1 = glm::vec3(x + width, y, z);     // bottom-left
        v2 = glm::vec3(x, y, z);             // bottom-right
        v3 = glm::vec3(x + width, y + height, z); // top-left
        v4 = glm::vec3(x, y + height, z);    // top-right
        break;
    case 5: // Front face (+Z)
        v1 = glm::vec3(x, y, z + 1);         // bottom-left
        v2 = glm::vec3(x + width, y, z + 1); // bottom-right
        v3 = glm::vec3(x, y + height, z + 1); // top-left
        v4 = glm::vec3(x + width, y + height, z + 1); // top-right
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
    int layer = Atlas::getLayer(quad.type, quad.direction);

	// UV mapping
    glm::vec3 uv1 = glm::vec3(0.0f, 0.0f, layer);        // bottom-left
    glm::vec3 uv2 = glm::vec3(width, 0.0f, layer);       // bottom-right
    glm::vec3 uv3 = glm::vec3(0.0f, height, layer);      // top-left
    glm::vec3 uv4 = glm::vec3(width, height, layer);     // top-right

    textures.push_back(uv1);
    textures.push_back(uv2);
    textures.push_back(uv3);
    textures.push_back(uv4);


	// Add ambient occlusion values
    if (m_world->useAmbientOcclusion)
    {
	    ao.push_back(quad.ao[0]);
	    ao.push_back(quad.ao[1]);
	    ao.push_back(quad.ao[2]);
	    ao.push_back(quad.ao[3]);

        float diagA = quad.ao[0] + quad.ao[3];
        float diagB = quad.ao[1] + quad.ao[2];
        bool flip = diagA > diagB;

        if (!flip) 
        {
            indices.push_back(startIndex);     // v1
            indices.push_back(startIndex + 1); // v2
            indices.push_back(startIndex + 2); // v3
            indices.push_back(startIndex + 2); // v3
            indices.push_back(startIndex + 1); // v2
            indices.push_back(startIndex + 3); // v4
        }
        else 
        {
            indices.push_back(startIndex);     // v1
            indices.push_back(startIndex + 1); // v2
            indices.push_back(startIndex + 3); // v4
            indices.push_back(startIndex);     // v1
            indices.push_back(startIndex + 3); // v4
            indices.push_back(startIndex + 2); // v3
        }
    }
    else
    {
        indices.push_back(startIndex);     // v1
        indices.push_back(startIndex + 1); // v2
        indices.push_back(startIndex + 2); // v3
        indices.push_back(startIndex + 2); // v3
        indices.push_back(startIndex + 1); // v2
        indices.push_back(startIndex + 3); // v4
    }
}

int Chunk::getSurfaceY(int x, int z) const
{
    for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
        if (cubes[x][y][z] != BlockType::None) return y;
    }
    return 0;
}

void Chunk::plantTree(int x, int y, int z)
{
	// Trunk
	for (int i = 0; i < 3; ++i) {
		if (y + i < CHUNK_HEIGHT) {
			cubes[x][y + i][z] = BlockType::Tree;
		}
	}

    // Leaves
    int leafStartY = y + 2;
    int leafLayers = 2;
    int baseRadius = 2;  

    for (int i = 0; i < leafLayers; ++i) {
        int dy = leafStartY + i;

        int radius = baseRadius - i;
        if (radius < 0) radius = 0;

        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dz = -radius; dz <= radius; ++dz) {
                if (dx * dx + dz * dz <= radius * radius) {
                    int nx = x + dx;
                    int nz = z + dz;
                    if (nx >= 0 && nx < CHUNK_SIZE
                        && nz >= 0 && nz < CHUNK_SIZE
                        && dy < CHUNK_HEIGHT)
                    {
                        if (cubes[nx][dy][nz] != BlockType::Tree) {
                            cubes[nx][dy][nz] = BlockType::Leaves;
                        }
                    }
                }
            }
        }
    }

}

BlockType Chunk::getNeighborType(const glm::ivec3& pos, const glm::ivec3& dir) const
{
    BlockType type = BlockType::None;
    int nx = pos.x + dir.x;
    int ny = pos.y + dir.y;
    int nz = pos.z + dir.z;

    // if neighbor is out of chunk
    if (nx < 0 || nx >= CHUNK_SIZE ||
        ny < 0 || ny >= CHUNK_HEIGHT ||
        nz < 0 || nz >= CHUNK_SIZE)
    {
        // Neighbor is outside this chunk - check adjacent chunk
        Chunk* neighbor = nullptr;
        glm::ivec3 neighborPos = glm::ivec3(nx, ny, nz);
        glm::ivec3 neighborDelta = glm::ivec3(0, 0, 0);

        if (nx < 0) {
            neighborDelta.x = -CHUNK_SIZE;
            neighborPos.x = nx + CHUNK_SIZE; // This will be CHUNK_SIZE-1 when nx=-1
        }
        else if (nx >= CHUNK_SIZE) {
            neighborDelta.x = CHUNK_SIZE;
            neighborPos.x = nx - CHUNK_SIZE; // This will be 0 when nx=CHUNK_SIZE
        }

        if (ny < 0) {
            neighborDelta.y = -CHUNK_HEIGHT;
            neighborPos.y = ny + CHUNK_HEIGHT;
        }
        else if (ny >= CHUNK_HEIGHT) {
            neighborDelta.y = CHUNK_HEIGHT;
            neighborPos.y = ny - CHUNK_HEIGHT;
        }

        if (nz < 0) {
            neighborDelta.z = -CHUNK_SIZE;
            neighborPos.z = nz + CHUNK_SIZE;
        }
        else if (nz >= CHUNK_SIZE) {
            neighborDelta.z = CHUNK_SIZE;
            neighborPos.z = nz - CHUNK_SIZE;
        }

        neighbor = m_world->getChunk(m_x + neighborDelta.x, m_y + neighborDelta.y, m_z + neighborDelta.z);
        if (neighbor) {
            type = neighbor->cubes[neighborPos.x][neighborPos.y][neighborPos.z];
        }
        else {
            type = BlockType::None;
        }
    }
    else {
        // Neighbor is within this chunk
        type = cubes[nx][ny][nz];
    }
    return type;
}

inline bool Chunk::isBlockFaceVisible(int x, int y, int z, const glm::ivec3& dir, BlockType faceType) const
{
    if (faceType == BlockType::None) return false;

	BlockType neighborBlockType = getNeighborType(glm::ivec3(x, y, z), dir);
	if (faceType == BlockType::Water)
	{
		return neighborBlockType == BlockType::None;
	}

	return neighborBlockType == BlockType::None || isTransparentBlock(neighborBlockType);
}

void Chunk::draw() const
{
	if (m_activeMesh) {
        m_activeMesh->draw();
	}
}

void Chunk::drawTransparent() const
{
    if (m_activeTransparentMesh) {
        m_activeTransparentMesh->draw();
    }
}

std::array<float, 4> Chunk::getAmbientOcclusion(const glm::ivec3& pos, const glm::ivec3& dir) const {
    int face = Atlas::faceIndexForDir(dir);
    std::array<float, 4> outAo;

    for (int v = 0; v < 4; ++v) {
        // which of the eight neighbours to test:
		glm::ivec3 aoIndices = m_neighborFaceIndices[v];

		BlockType neighborTypeX = getNeighborType(pos, m_faceAos[face][aoIndices.x]);
		BlockType neighborTypeZ = getNeighborType(pos, m_faceAos[face][aoIndices.z]);
		BlockType neighborTypeY = getNeighborType(pos, m_faceAos[face][aoIndices.y]);
		int s1 = neighborTypeX != BlockType::None && neighborTypeX != BlockType::Water;
		int s2 = neighborTypeZ != BlockType::None && neighborTypeZ != BlockType::Water;
		int c = neighborTypeY != BlockType::None && neighborTypeY != BlockType::Water;

        int state = (s1 + s2 == 2)
            ? 0
            : 3 - (s1 + s2 + c);

        outAo[v] = m_aoValues[state];
    }
    return outAo;
}