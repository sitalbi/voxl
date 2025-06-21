#pragma once

#include "subsystem.h"
#include "camera.h"
#include "chunk.h"
#include <memory>
#include <player.h>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <set>

struct ChunkHash {
	std::size_t operator()(const glm::ivec3& pos) const {
		return std::hash<int>()(pos.x) ^
			(std::hash<int>()(pos.y) << 1) ^
			(std::hash<int>()(pos.z) << 2);
	}
};

class World : public ISubsystem
{
public:
	World();
	~World() override;


	virtual bool init() override;
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;

	void loadChunks(glm::vec3 playerPosition);
	void unloadChunks(glm::vec3 playerPosition);

	void setPlayer(Player* player) {
		m_player = player;
	}

	void addChunk(Chunk* chunk) {
		glm::ivec3 pos(chunk->getPosition());
		auto it = m_chunks.find(pos);
		if (it == m_chunks.end()) {
			m_chunks[pos] = chunk; // Store the Chunk pointer
			m_updateList.insert(chunk); // Add to update list
		}
		else {
			std::cerr << "A Chunk already exists at this location in the world!" << std::endl;
		}
	}

	void removeChunk(Chunk* chunk) {
		auto it = m_chunks.find(glm::ivec3(chunk->getPosition()));
		if (it != m_chunks.end()) {
			delete it->second; // Delete the Chunk pointer
			m_chunks.erase(it);
		}
		else {
			std::cerr << "Chunk not found in world!" << std::endl;
		}
	}

	const Chunk* getChunk(int x, int y, int z) const;

	std::unordered_map<glm::ivec3, Chunk*, ChunkHash>& getChunks() { return m_chunks; }
	Player* getPlayer() const { return m_player; }

private:
	int m_meshesGenerated = 0;
	int m_maxMeshesPerFrame = 2;

	Player* m_player;

	std::set<Chunk*> m_updateList; 

	// For the moment we just store a single cube to test
	std::unique_ptr<Chunk> m_chunk;

	//std::vector<Chunk*> m_chunks;
	std::unordered_map<glm::ivec3, Chunk*, ChunkHash> m_chunks;

};
