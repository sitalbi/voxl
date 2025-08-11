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
#include <skybox.h>

struct SkyPalette {
	glm::vec3 horizon; 
	glm::vec3 zenith;
};

static inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t) { 
	return a + (b - a) * t; 
}

static inline SkyPalette Lerp(const SkyPalette& a, const SkyPalette& b, float t) {
	return { Lerp(a.horizon,b.horizon,t), Lerp(a.zenith,b.zenith,t) };
}

class World : public ISubsystem
{

public:
static const int NUM_CHUNK_PER_FRAME = 1;
static const int CHUNK_LOAD_RADIUS = 8;
static const size_t WORKER_COUNT = 4;

	World();
	~World() override;


	virtual bool init() override;
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;


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

	float getLightIntensity() const { return m_lightIntensity; }
	SkyPalette getSkyColor() const { return m_skyColor; }
	glm::vec3 getSunPosition() const { return m_sunPosition; }

	void setAmbientOcclusion();

	fnl_state noise; 

	bool useAmbientOcclusion = true;
	
	float dayTimer = 0.0f; // Timer for the day cycle
	float dayLength = 0.0f; 
	inline float time01() { return float(ticks) / kTicksPerDay; }


private:
	int m_chunksProcessed = 0;

	Player* m_player;

	std::set<Chunk*> m_chunksToGenerate; 
	std::unordered_set<glm::ivec3> m_chunksToRemove; 
	std::set<Chunk*> m_chunkMeshesToSetup; 
	std::set<Chunk*> m_chunksToRender;

	// std::vector<Chunk*> m_chunks;
	std::unordered_map<glm::ivec3, Chunk*> m_chunks;

	// Lighting
	float m_lightIntensity = 1.0f;
	
	SkyPalette m_skyNight{ {0.035f, 0.008f, 0.141f}, {0.0f,0.0f,0.0f} };
	SkyPalette m_skySunrise{ {0.659f, 0.0f, 0.659f}, {0.0f, 0.706f, 1.0f} };
	SkyPalette m_skyDay{ {0.655f, 0.898f, 1.0f}, {0.0f, 0.706f, 1.0f} };
	SkyPalette m_skySunset = { {1.0f, 0.7f, 0.482f}, {0.0f, 0.0f, 0.0f} };
	SkyPalette m_skyColor = m_skyDay; // Default to day color

	enum Phase { Day, Sunset, Night, Sunrise } phase = Day;
	float phaseTimer = 0.0f;    

	float dayDuration = 10.0f; 
	float transitionDuration = 2.5f; 
	float nightDuration = 10.0f;  

	const float maxLight = 1.0f;
	const float minLight = 0.05f;

	glm::vec3 m_sunPosition = glm::vec3(0.0f, 1.0f, 0.0f); 

	// Time
	uint32_t ticks = 0;
	static constexpr uint32_t kTicksPerDay = 24000;
	static constexpr float    kSecondsPerDay = 1200.0f; // 20 real min/day

	void advanceTime(float deltaTime);

	// Multi-threading
	ThreadPool                   meshThreadPool{ WORKER_COUNT };
	std::mutex                   meshResultMutex;
	std::queue<Chunk*>           meshResults;
	std::unordered_set<Chunk*>   meshEnqueued;

	void loadChunks(glm::vec3 playerPosition);
	void unloadChunks(glm::vec3 playerPosition);
	void generateChunks();
	void setupChunks();
	void removeChunks();

	void updateLighting(float deltaTime);


};
