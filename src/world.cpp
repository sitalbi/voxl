#include "world.h"
#include "application.h"

World::World()
{
}

World::~World()
{
}

bool World::init()
{
	if (m_isInitialized)
	{
		return true;
	}

	m_chunk = std::make_unique<Chunk>();
	m_chunk->generate();
	
	return true;
}

void World::update(float deltaTime)
{
	// TODO: update chunk
}

void World::shutdown()
{

}

