#pragma once

#include "subsystem.h"
#include "camera.h"
#include "chunk.h"
#include <memory>
#include <player.h>

class World : public ISubsystem
{
public:
	World();
	~World() override;


	virtual bool init() override;
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;

	void setPlayer(std::shared_ptr<Player> player) {
		m_player = player;
	}
	//std::unique_ptr<Cube>& getCube() { return m_cube; }
	std::unique_ptr<Chunk>& getChunk() { return m_chunk; }
	std::shared_ptr<Player> getPlayer() const { return m_player; }

private:

	std::shared_ptr<Player> m_player;

	// Chunk array

	// For the moment we just store a single cube to test
	std::unique_ptr<Chunk> m_chunk;

};
