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
		return true;
	}

	int numChunks = 3; 

	// intit chunks 
	for (int x = 0; x < numChunks; ++x)
	{
		for (int z = 0; z < numChunks; ++z)
		{
			Chunk* newChunk = new Chunk(x, 0, z, this);
			m_chunks.insert({ glm::ivec3(x, 0, z), newChunk });
			newChunk->generate();
			m_updateList.push_back(newChunk);
		}
	}
	
	return true;
}

void World::update(float deltaTime)
{
	// update chunks
	for (auto chunk : m_updateList)
	{
		chunk->generateGreedyMesh();
	}

	if (m_updateList.size() > 0)
	{
		m_updateList.clear();
	}
}

void World::shutdown()
{

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

