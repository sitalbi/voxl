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

	m_cube = std::make_unique<Cube>(BlockType::Dirt, glm::vec3(0.0f, 0.0f, 0.0f));
	
	return true;
}

void World::update(float deltaTime)
{
	// TODO: update player when player will be impl
}

void World::shutdown()
{

}

