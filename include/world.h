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
#include <FastNoiseLite.h>
#include "thread.h"



class World : public ISubsystem
{
static const int NUM_CHUNK_PER_FRAME = 1;
static const int CHUNK_LOAD_RADIUS = 3;
static const size_t WORKER_COUNT = 4;

public:
	World();
	~World() override;


	virtual bool init() override;
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;

	void loadChunks(glm::vec3 playerPosition);
	void unloadChunks(glm::vec3 playerPosition);
	void generateChunks();
	void setupChunks();
	void removeChunks();
	void updateChunk(Chunk* chunk);

	void setPlayer(Player* player) {
		m_player = player;
	}

	void addChunk(Chunk* chunk) {
		glm::ivec3 pos(chunk->getWorldPosition());
		auto it = m_chunks.find(pos);
		if (it == m_chunks.end()) {
			m_chunks[pos] = chunk; // Store the Chunk pointer
			m_chunksToGenerate.insert(chunk); // Add to update list
		}
		else {
			std::cerr << "A Chunk already exists at this location in the world!" << std::endl;
		}
	}

	void removeChunk(Chunk* chunk) {
		auto it = m_chunks.find(glm::ivec3(chunk->getWorldPosition()));
		if (it != m_chunks.end()) {
			delete it->second; // Delete the Chunk pointer
			m_chunks.erase(it);
		}
		else {
			std::cerr << "Chunk not found in world!" << std::endl;
		}
	}

	Chunk* getChunk(int x, int y, int z) const;
	Chunk* getChunkWorldPos(float x, float y, float z) const;

	BlockType getBlockTypeWorld(const glm::ivec3& worldPos) const;

	bool isSolidBlock(int x, int y, int z) const;

	std::unordered_map<glm::ivec3, Chunk*>& getChunks() { return m_chunks; }
	std::set<Chunk*>& getRenderList() { return m_chunksToRender; }
	Player* getPlayer() const { return m_player; }

	void setAmbientOcclusion();

	fnl_state noise; 

	bool useAmbientOcclusion = true;
private:
	int m_chunksProcessed = 0;

	Player* m_player;

	std::set<Chunk*> m_chunksToGenerate; 
	std::unordered_set<glm::ivec3> m_chunksToRemove; 
	std::set<Chunk*> m_chunkMeshesToSetup; 
	std::set<Chunk*> m_chunksToRender;

	// For the moment we just store a single cube to test
	std::unique_ptr<Chunk> m_chunk;

	// std::vector<Chunk*> m_chunks;
	std::unordered_map<glm::ivec3, Chunk*> m_chunks;

	// Multi-threading
	ThreadPool                   meshThreadPool{ WORKER_COUNT };
	std::mutex                   meshResultMutex;
	std::queue<Chunk*>           meshResults;
	std::unordered_set<Chunk*>   meshEnqueued;

};
