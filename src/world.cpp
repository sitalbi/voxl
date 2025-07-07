#include "world.h"
#include "application.h"
#include <iostream>



World::World()
{
}

World::~World()
{
	for (auto chunk : m_chunks)
	{
		delete chunk.second; // Delete the Chunk pointer
	}
	m_chunks.clear();
}

bool World::init()
{
	if (m_isInitialized)
	{
		return false;
	}
	return true;
}

void World::update(float deltaTime)
{
	loadChunks(m_player->getWorldPosition());

	generateChunks();

	setupChunks();

	// todo: maybe make a render list for chunks to render

	unloadChunks(m_player->getWorldPosition());

	removeChunks();
}

void World::shutdown()
{

}

void World::loadChunks(glm::vec3 playerPosition)
{
	// Load chunks around the player
	int playerChunkX = static_cast<int>(playerPosition.x) / Chunk::CHUNK_SIZE;
	int playerChunkZ = static_cast<int>(playerPosition.z) / Chunk::CHUNK_SIZE;


	for (int x = playerChunkX - CHUNK_LOAD_RADIUS; x < playerChunkX + CHUNK_LOAD_RADIUS; x++)
	{
		for (int z = playerChunkZ - CHUNK_LOAD_RADIUS; z < playerChunkZ + CHUNK_LOAD_RADIUS; z++)
		{
			glm::ivec3 chunkPos(x, 0, z);
			if (m_chunks.find(chunkPos) == m_chunks.end()) {
				Chunk* chunk = new Chunk(x, 0, z, this);
				chunk->load();
				m_chunks[chunkPos] = chunk;
				m_chunksToGenerate.insert(chunk);

				// Update neighboring chunks
				std::vector<glm::ivec3> neighborsPos = {
					glm::ivec3(x - 1, 0, z),
					glm::ivec3(x + 1, 0, z),
					glm::ivec3(x, 0, z - 1),
					glm::ivec3(x, 0, z + 1)
				};

				for (const auto& pos : neighborsPos) {
					if (m_chunks.find(pos) != m_chunks.end()) {
						Chunk* neighborChunk = m_chunks[pos];
						if (neighborChunk) {
							m_chunksToGenerate.insert(neighborChunk);
						}
					}
				}
			}
		}
	}
}

void World::unloadChunks(glm::vec3 playerPosition)
{
	// Unload chunks that are too far away from the player
	int playerChunkX = static_cast<int>(playerPosition.x) / Chunk::CHUNK_SIZE;
	int playerChunkZ = static_cast<int>(playerPosition.z) / Chunk::CHUNK_SIZE;
	for (const auto& chunk : m_chunks)
	{
		int chunkX = chunk.second->getWorldPosition().x / Chunk::CHUNK_SIZE;
		int chunkZ = chunk.second->getWorldPosition().z / Chunk::CHUNK_SIZE;
		if (abs(chunkX - playerChunkX) > CHUNK_LOAD_RADIUS * 1.75f || abs(chunkZ - playerChunkZ) > CHUNK_LOAD_RADIUS * 1.75f)
		{
			m_chunksToRemove.insert(chunk.first);
		}
	}
}

void World::generateChunks()
{
	m_chunksProcessed = 0;
	auto it = m_chunksToGenerate.begin();
	while (it != m_chunksToGenerate.end() && m_chunksProcessed < NUM_CHUNK_PER_FRAME)
	{
		Chunk* chunk = *it;
		if (chunk) {
			chunk->generateMeshData();
			m_chunkMeshesToSetup.insert(chunk);
			it = m_chunksToGenerate.erase(it); 
			m_chunksProcessed++;
		}
		else {
			it++;
		}
	}
}

void World::setupChunks()
{
	m_chunksProcessed = 0;
	auto it = m_chunkMeshesToSetup.begin();
	while (it != m_chunkMeshesToSetup.end() && m_chunksProcessed < NUM_CHUNK_PER_FRAME)
	{
		Chunk* chunk = *it;
		if (chunk) {
			chunk->getMesh()->setupMesh();
			chunk->getTransparentMesh()->setupMesh();
			it = m_chunkMeshesToSetup.erase(it); 
			m_chunksProcessed++;
		}
		else {
			it++;
		}
	}
}

void World::removeChunks()
{
	for (auto const& coord : m_chunksToRemove)
	{
		// 1) find the map entry
		auto it = m_chunks.find(coord);
		if (it == m_chunks.end())
			continue;

		Chunk* chunk = it->second;

		// 2) cancel any pending work for that chunk
		m_chunksToGenerate.erase(chunk);
		m_chunkMeshesToSetup.erase(chunk);

		// 3) remove from the map
		m_chunks.erase(it);

		// 4) finally delete its memory
		delete chunk;
	}

	m_chunksToRemove.clear();
}

void World::updateChunk(Chunk* chunk)
{
	glm::vec3 chunkPos = chunk->getPositionGrid();
	m_chunksToGenerate.insert(chunk);

	// Update neighboring chunks
	std::vector<glm::ivec3> neighborsPos = {
		glm::ivec3(chunkPos.x - 1, 0, chunkPos.z),
		glm::ivec3(chunkPos.x + 1, 0, chunkPos.z),
		glm::ivec3(chunkPos.x, 0, chunkPos.z - 1),
		glm::ivec3(chunkPos.x, 0, chunkPos.z + 1)
	};

	for (const auto& pos : neighborsPos) {
		if (m_chunks.find(pos) != m_chunks.end()) {
			Chunk* neighborChunk = m_chunks[pos];
			if (neighborChunk) {
				m_chunksToGenerate.insert(neighborChunk);
			}
		}
	}
}


Chunk* World::getChunk(int x, int y, int z) const
{
	int chunkX = static_cast<int>(std::floor(x / Chunk::CHUNK_SIZE));
	int chunkY = static_cast<int>(std::floor(y / Chunk::CHUNK_HEIGHT));
	int chunkZ = static_cast<int>(std::floor(z / Chunk::CHUNK_SIZE));


	glm::ivec3 chunkPos(chunkX, chunkY, chunkZ);
	auto it = m_chunks.find(chunkPos);
	if (it != m_chunks.end()) {
		return it->second;
	}
	return nullptr;
}

Chunk* World::getChunkWorldPos(float x, float y, float z) const
{
	int wx = static_cast<int>(std::floor(x));
	int wy = static_cast<int>(std::floor(y));
	int wz = static_cast<int>(std::floor(z));

	int chunkX = static_cast<int>(std::floor(wx / float(Chunk::CHUNK_SIZE)));
	int chunkY = static_cast<int>(std::floor(wy / float(Chunk::CHUNK_HEIGHT)));
	int chunkZ = static_cast<int>(std::floor(wz / float(Chunk::CHUNK_SIZE)));

	glm::ivec3 key(chunkX, chunkY, chunkZ);
	auto it = m_chunks.find(key);
	if (it != m_chunks.end()) {
		return it->second;
	}
	return nullptr;
}

BlockType World::getBlockTypeWorld(const glm::ivec3& worldPos) const {
	BlockType type = BlockType::None;

	Chunk* chunk = getChunkWorldPos(worldPos.x, worldPos.y, worldPos.z);

	return chunk ? chunk->getBlockTypeWorldPos(worldPos) : type;
}

bool World::isSolidBlock(int x, int y, int z) const
{
	Chunk* chunk = getChunkWorldPos(x, y, z);
	if (chunk) {
		glm::vec3 localPos = glm::vec3(x, y, z) - chunk->getWorldPosition();
		glm::ivec3 localBlockPos = glm::floor(localPos);
		return chunk->cubes[localBlockPos.x][localBlockPos.y][localBlockPos.z] != BlockType::None &&
			chunk->cubes[localBlockPos.x][localBlockPos.y][localBlockPos.z] != BlockType::Water;
	}
	return false;
}
