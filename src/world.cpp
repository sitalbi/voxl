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

	noise = fnlCreateState();
	return true;
}

void World::update(float deltaTime)
{
	loadChunks(m_player->getWorldPosition());

	generateChunks();

	setupChunks();

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
	for (Chunk* chunk : m_chunksToGenerate) {
		if (meshEnqueued.insert(chunk).second) {
			meshThreadPool.enqueue([this, chunk]() {
				chunk->generateMeshData();
				{
					std::lock_guard<std::mutex> lock(meshResultMutex);
					meshResults.push(chunk);
				}
				});
		}
	}
	m_chunksToGenerate.clear();
}

void World::setupChunks()
{
	int processed = 0;
	while (processed < NUM_CHUNK_PER_FRAME) {
		Chunk* chunk = nullptr;
		{   
			// pop one result
			std::lock_guard<std::mutex> lock(meshResultMutex);
			if (meshResults.empty()) break;
			chunk = meshResults.front();
			meshResults.pop();
		}
		if (!chunk) continue;

		chunk->getMesh()->setupMesh();
		chunk->getTransparentMesh()->setupMesh();
		chunk->swapMeshes(); 

		m_chunksToRender.insert(chunk); // TODO: sort render list by distance and angle to player (closest and visible chunks first)

		meshEnqueued.erase(chunk);
		++processed;
	}
}

void World::removeChunks()
{
	for (auto const& coord : m_chunksToRemove)
	{
		auto it = m_chunks.find(coord);
		if (it == m_chunks.end())
			continue;

		Chunk* chunk = it->second;

		m_chunksToGenerate.erase(chunk);
		m_chunksToRender.erase(chunk);

		m_chunks.erase(it);

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

void World::setAmbientOcclusion()
{
	useAmbientOcclusion = !useAmbientOcclusion;
	for (auto& chunkPair : m_chunks) {
		Chunk* chunk = chunkPair.second;
		if (chunk) {
			m_chunksToGenerate.insert(chunk);
		}
	}
}
