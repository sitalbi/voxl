#include "player.h"
#include "voxl.h"
#include <glm/gtc/matrix_transform.hpp> 
#include <unordered_set>
#include "world.h"
#include <iostream>

Player::Player(glm::vec3 position, World* world) : m_position(position)
{
	m_world = world;
	m_camera = std::make_unique<Camera>(window_width, window_height, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
	m_speed = m_defaultSpeed;

	updateCamera();

	m_playerForward = m_camera->getForward();
	m_playerUp = m_camera->getUp();

	m_isFlying = true;
	if (m_isFlying) {
		m_speed = m_defaultSpeed * 2.0f;
		m_speedMultiplier = m_defaultSpeedMultiplier * 2.0f;
		m_defaultSpeedMultiplier = 2.0f;
	}
	else {
		m_speed = m_defaultSpeed;
		m_speedMultiplier = m_defaultSpeedMultiplier;
		m_defaultSpeedMultiplier = 1.0f;
	}
}

void Player::update(float deltaTime)
{
	m_blockFound = rayCast(10.0f, m_blockPosition, m_blockNormal);

	// Process user input to update velocity
	processInput(glfwGetCurrentContext(), deltaTime);

	if (!m_isFlying) {
		// Apply gravity if not grounded
		if (!m_isGrounded) {
			m_verticalVelocity += m_gravity * deltaTime;
		}
		else {
			m_verticalVelocity = 0.0f;
		}

		m_velocity.y = m_verticalVelocity;
	}

	if (isSprinting) {
		m_speedMultiplier = m_defaultSpeedMultiplier * 1.5f;
	}
	else {
		m_speedMultiplier = m_defaultSpeedMultiplier;
	}

	m_position.x += m_velocity.x * deltaTime;
	if (!m_isFlying) handleCollisions(m_velocity.x, 0.0f, 0.0f);
	m_position.z += m_velocity.z * deltaTime;
	if (!m_isFlying) handleCollisions(0.0f, 0.0f, m_velocity.z);

	if (!m_isFlying) updateGrounded();

	m_position.y += m_velocity.y * deltaTime;
	if (!m_isFlying) handleVerticalCollisions();

	// Update camera position
	updateCamera();

	// Update forward direction of the player
	m_playerForward = glm::normalize(glm::vec3(m_camera->getForward().x, 0.0f, m_camera->getForward().z));
}

void Player::processInput(GLFWwindow* window, float deltaTime)
{
    m_velocity = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_velocity += m_playerForward * m_speed * m_speedMultiplier;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_velocity -= m_playerForward * m_speed * m_speedMultiplier;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_velocity -= m_camera->getRight() * m_speed * m_speedMultiplier;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_velocity += m_camera->getRight() * m_speed * m_speedMultiplier;
    }

    if (m_isFlying) {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            m_velocity += m_playerUp * m_speed * m_speedMultiplier;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            m_velocity -= m_playerUp * m_speed * m_speedMultiplier;
        }
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && m_isGrounded) {
            m_verticalVelocity = m_jumpVelocity;
            m_isGrounded = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        isSprinting = true;
    }
    else {
        isSprinting = false;
    }

    onPressedKey(GLFW_KEY_F1, [&]() {
        wireframeMode = !wireframeMode;
        });

    onPressedKey(GLFW_KEY_F2, [&]() {
        m_isFlying = !m_isFlying;
        if (m_isFlying) {
            m_speed = m_defaultSpeed * 2.0f;
            m_speedMultiplier = m_defaultSpeedMultiplier * 2.0f;
            m_defaultSpeedMultiplier = 2.0f;
        }
        else {
            m_speed = m_defaultSpeed;
            m_speedMultiplier = m_defaultSpeedMultiplier;
            m_defaultSpeedMultiplier = 1.0f;
        }
        });


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // If mouse left is pressed
    onPressedMouse(GLFW_MOUSE_BUTTON_LEFT, [&]() {
        if (m_blockFound) {
            glm::vec3 newBlockPosition = m_blockPosition + m_blockNormal;

            Chunk* chunk = m_world->getChunkWorldPos(newBlockPosition.x, newBlockPosition.y, newBlockPosition.z);

            if (chunk != nullptr) {
                int blockX = glm::mod(glm::floor(newBlockPosition.x), static_cast<float>(Chunk::CHUNK_SIZE));
                int blockY = glm::mod(glm::floor(newBlockPosition.y), static_cast<float>(Chunk::CHUNK_HEIGHT));
                int blockZ = glm::mod(glm::floor(newBlockPosition.z), static_cast<float>(Chunk::CHUNK_SIZE));

                glm::ivec3 localBlockPos = glm::ivec3(blockX, blockY, blockZ);
                if (localBlockPos.x >= 0 && localBlockPos.y >= 0 && localBlockPos.z >= 0 &&
                    localBlockPos.x < Chunk::CHUNK_SIZE && localBlockPos.y < Chunk::CHUNK_HEIGHT && localBlockPos.z < Chunk::CHUNK_SIZE) {
                    chunk->setBlockType(localBlockPos.x, localBlockPos.y, localBlockPos.z, BlockType::Dirt);

                    m_world->updateChunk(chunk);
                }
            }
        }
        });

    // If mouse right is pressed
    onPressedMouse(GLFW_MOUSE_BUTTON_RIGHT, [&]() {
        if (m_blockFound) {
            Chunk* chunk = m_world->getChunkWorldPos(m_blockPosition.x, m_blockPosition.y, m_blockPosition.z);

            if (chunk != nullptr) {
                int blockX = glm::mod(glm::floor(m_blockPosition.x), static_cast<float>(Chunk::CHUNK_SIZE));
                int blockY = glm::mod(glm::floor(m_blockPosition.y), static_cast<float>(Chunk::CHUNK_HEIGHT));
                int blockZ = glm::mod(glm::floor(m_blockPosition.z), static_cast<float>(Chunk::CHUNK_SIZE));

                glm::ivec3 localBlockPos = glm::ivec3(blockX, blockY, blockZ);
                if (localBlockPos.x >= 0 && localBlockPos.y >= 0 && localBlockPos.z >= 0 &&
                    localBlockPos.x < Chunk::CHUNK_SIZE && localBlockPos.y < Chunk::CHUNK_HEIGHT && localBlockPos.z < Chunk::CHUNK_SIZE) {
                    chunk->setBlockType(localBlockPos.x, localBlockPos.y, localBlockPos.z, BlockType::None);

                    m_world->updateChunk(chunk);
                }
            }
        }
        });
}

void Player::processMouseMovement(double xpos, double ypos)
{
    static double lastX = window_width / 2.0;
    static double lastY = window_height / 2.0;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    m_camera->processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void Player::updateCamera()
{
    m_cameraPosition = glm::vec3(m_position.x, m_position.y + m_height, m_position.z);
    m_camera->setPosition(m_cameraPosition);
}

void Player::updateGrounded()
{
    int x1 = int(std::floor(m_position.x - m_width));
    int x2 = int(std::floor(m_position.x + m_width));

    int y0 = int(std::floor(m_position.y - 0.05f));
    int z1 = int(std::floor(m_position.z - m_width));
    int z2 = int(std::floor(m_position.z + m_width));

    bool onFloor =
        m_world->isSolidBlock(x1, y0, z1) ||
        m_world->isSolidBlock(x2, y0, z1) ||
        m_world->isSolidBlock(x1, y0, z2) ||
        m_world->isSolidBlock(x2, y0, z2);

    m_isGrounded = onFloor;
}

void Player::handleCollisions(float dx, float dy, float dz)
{
    // normalize the direction
	glm::vec3 direction = glm::normalize(glm::vec3(dx, dy, dz));

    int x1 = int(floor(m_position.x - m_width));
    int x2 = int(floor(m_position.x + m_width));
    int z1 = int(floor(m_position.z - m_width));
    int z2 = int(floor(m_position.z + m_width));
    int x3 = glm::floor(m_position.x - direction.x - m_width);
    int y3 = glm::floor(m_position.y - direction.y);
    int z3 = glm::floor(m_position.z - direction.z - m_width);
    int x4 = glm::floor(m_position.x - direction.x + m_width);
    int y4 = glm::floor(m_position.y - direction.y + m_height);
    int z4 = glm::floor(m_position.z - direction.z + m_width);
    int y5 = glm::round(y3 + (y4 - y3) / 2); 

    // right
    if (m_world->isSolidBlock(x1, y3, z3) || m_world->isSolidBlock(x1, y3, z4) ||
        m_world->isSolidBlock(x1, y4, z3) || m_world->isSolidBlock(x1, y4, z4) ||
        m_world->isSolidBlock(x1, y5, z3) || m_world->isSolidBlock(x1, y5, z4)) {

        if (direction.x < 0) {
            m_position.x = x3 + m_width;
            m_velocity.x = 0.0f;
        }
    }

    // left
    if (m_world->isSolidBlock(x2, y3, z3) || m_world->isSolidBlock(x2, y3, z4) ||
        m_world->isSolidBlock(x2, y4, z3) || m_world->isSolidBlock(x2, y4, z4) ||
        m_world->isSolidBlock(x2, y5, z3) || m_world->isSolidBlock(x2, y5, z4)) {

        if (direction.x > 0) {
            m_position.x = (x4 + 1) - m_width - 0.01f;
            m_velocity.x = 0.0f;
        }
    }

    // forward
    if (m_world->isSolidBlock(x3, y3, z1) || m_world->isSolidBlock(x4, y3, z1) ||
        m_world->isSolidBlock(x4, y4, z1) || m_world->isSolidBlock(x3, y4, z1) ||
        m_world->isSolidBlock(x3, y5, z1) || m_world->isSolidBlock(x4, y5, z1)) {

        if (direction.z < 0) {
            m_position.z = z3 + m_width;
            m_velocity.z = 0.0f;
        }
    }

    // backward
    if (m_world->isSolidBlock(x3, y3, z2) || m_world->isSolidBlock(x4, y3, z2) ||
        m_world->isSolidBlock(x4, y4, z2) || m_world->isSolidBlock(x3, y4, z2) ||
        m_world->isSolidBlock(x3, y5, z2) || m_world->isSolidBlock(x4, y5, z2)) {

        if (direction.z > 0) {
            m_position.z = (z4 + 1) - m_width - 0.01f;
            m_velocity.z = 0.0f;
        }
    }
}

void Player::handleVerticalCollisions() {
    int x1 = int(floor(m_position.x - m_width));
    int x2 = int(floor(m_position.x + m_width));
    int y1 = int(floor(m_position.y));
    int y2 = int(floor(m_position.y + m_height));
    int z1 = int(floor(m_position.z - m_width));
    int z2 = int(floor(m_position.z + m_width));

    if (m_velocity.y < 0.0f) {
        bool hitFloor =
            m_world->isSolidBlock(x1, y1, z1) ||
            m_world->isSolidBlock(x2, y1, z1) ||
            m_world->isSolidBlock(x2, y1, z2) ||
            m_world->isSolidBlock(x1, y1, z2);

        if (hitFloor) {
            m_position.y = y1 + 1.0f;
            m_velocity.y = 0.0f;
        }
    }
    else if (m_velocity.y > 0.0f) {
        bool hitCeiling =
            m_world->isSolidBlock(x1, y2, z1) ||
            m_world->isSolidBlock(x2, y2, z1) ||
            m_world->isSolidBlock(x2, y2, z2) ||
            m_world->isSolidBlock(x1, y2, z2);

        if (hitCeiling) {
            m_position.y = y2 - m_height - 0.01f;
            m_velocity.y = 0.0f;
        }
    }
}


void Player::onPressedKey(int key, const std::function<void()>& callback)
{
    bool isPressed = glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS;
    if (isPressed && !m_keyStates[key]) {
        callback();
    }
    m_keyStates[key] = isPressed;
}

void Player::onPressedMouse(int button, const std::function<void()>& callback)
{
    bool isPressed = glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
    if (isPressed && !m_mouseButtonStates[button]) {
        callback();
    }
    m_mouseButtonStates[button] = isPressed;
}



bool Player::rayCast(float maxDistance, glm::vec3& outBlockPosition, glm::vec3& outNormal) const
{
    
    glm::vec3 rayOrigin = m_camera->getWorldPosition();
    glm::vec3 rayDirection = glm::normalize(m_camera->getForward());
    float step = 0.1f;

    glm::vec3 currentPos;
    for (float distance = 0.0f; distance < maxDistance; distance += step) {
        currentPos = rayOrigin + rayDirection * distance;

        Chunk* chunk = m_world->getChunkWorldPos(currentPos.x, currentPos.y, currentPos.z);
        if (chunk == nullptr) {
            return false;
        }

        int blockX = glm::mod(glm::floor(currentPos.x), static_cast<float>(Chunk::CHUNK_SIZE));
        int blockY = glm::mod(glm::floor(currentPos.y), static_cast<float>(Chunk::CHUNK_HEIGHT));
        int blockZ = glm::mod(glm::floor(currentPos.z), static_cast<float>(Chunk::CHUNK_SIZE));

        glm::ivec3 blockPos = glm::ivec3(blockX, blockY, blockZ);

        if (m_nonSelectableBlockTypes.find(chunk->cubes[blockPos.x][blockPos.y][blockPos.z]) == m_nonSelectableBlockTypes.end()) {
            glm::vec3 blockCenter = chunk->getWorldPosition() + glm::vec3(blockPos) + glm::vec3(0.5f);
            glm::vec3 delta = currentPos - blockCenter;

            if (fabs(delta.x) > fabs(delta.y) && fabs(delta.x) > fabs(delta.z)) {
                outNormal = glm::vec3(delta.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
            }
            else if (fabs(delta.y) > fabs(delta.x) && fabs(delta.y) > fabs(delta.z)) {
                outNormal = glm::vec3(0.0f, delta.y > 0 ? 1.0f : -1.0f, 0.0f);
            }
            else {
                outNormal = glm::vec3(0.0f, 0.0f, delta.z > 0 ? 1.0f : -1.0f);
            }

            outBlockPosition = blockCenter;
            return true;
        }
    }
    return false;
}