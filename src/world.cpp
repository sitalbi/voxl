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
		delete chunk;
	}
	m_chunks.clear();
}

bool World::init()
{
	if (m_isInitialized)
	{
		return true;
	}

	// intit chunks 
	for (int x = 0; x < 3; ++x)
	{
		for (int z = 0; z < 3; ++z)
		{
			m_chunks.push_back(new Chunk(x, 0, z, this));
			m_chunks.back()->generate();
		}
	}
	/*m_chunk = std::make_unique<Chunk>(0, 0, 0, this);
	m_chunk->generate();*/
	
	return true;
}

void World::update(float deltaTime)
{
	// TODO: update chunk
}

void World::shutdown()
{

}

