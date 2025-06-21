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
	loadChunks(m_player->getPosition());
	unloadChunks(m_player->getPosition());

	// update chunks
	for (auto& chunk : m_updateList)
	{
		if (chunk) {
			chunk->generateGreedyMesh();
		}
	}
	m_updateList.clear();
}

void World::shutdown()
{

}

void World::loadChunks(glm::vec3 playerPosition)
{
	// Load chunks around the player
	int playerChunkX = static_cast<int>(playerPosition.x) / Chunk::CHUNK_SIZE;
	int playerChunkZ = static_cast<int>(playerPosition.z) / Chunk::CHUNK_SIZE;


	for (int x = playerChunkX - 8; x < playerChunkX + 8; x++)
	{
		for (int z = playerChunkZ - 8; z < playerChunkZ + 8; z++)
		{
			glm::ivec3 chunkPos(x, 0, z);
			if (m_chunks.find(chunkPos) == m_chunks.end()) {
				Chunk* chunk = new Chunk(x, 0, z, this);
				chunk->generate();
				m_chunks[chunkPos] = chunk;
				m_updateList.insert(chunk);

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
							m_updateList.insert(neighborChunk);
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
	std::vector<glm::ivec3> chunksToRemove;
	for (auto& chunk : m_chunks)
	{
		int chunkX = chunk.second->getPosition().x / Chunk::CHUNK_SIZE;
		int chunkZ = chunk.second->getPosition().z / Chunk::CHUNK_SIZE;
		if (abs(chunkX - playerChunkX) > 8 || abs(chunkZ - playerChunkZ) > 8)
		{
			chunksToRemove.push_back(glm::ivec3(chunkX, 0, chunkZ));
		}
	}
	for (auto& chunk : chunksToRemove)
	{
		m_chunks.erase(chunk);
	}
}

const Chunk* World::getChunk(int x, int y, int z) const
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

