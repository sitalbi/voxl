#pragma once

#include "camera.h"
#include "cube.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <memory>
#include <unordered_map>
#include <functional>


class Player {
public:
    Player(glm::vec3 position);

    void update(float deltaTime);
    void processInput(GLFWwindow* window, float deltaTime);
    void processMouseMovement(double xpos, double ypos);

    //bool rayCast(const ChunkManager& chunkManager, float maxDistance, glm::vec3& outBlockPosition, glm::vec3& outNormal) const;

    const glm::vec3& getPosition() const { return m_position; }

	glm::mat4 getView() const {
		return m_camera->getViewMatrix();
	}

	glm::mat4 getProjection() const {
		return m_camera->getProjectionMatrix();
	}

    bool wireframeMode = false;

private:
    std::unordered_map<int, bool> m_keyStates;
    std::unordered_map<int, bool> m_mouseButtonStates;

    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_cameraPosition;
    glm::vec3 m_velocity;
    glm::vec3 m_playerForward;
    glm::vec3 m_playerUp;

    glm::vec3 m_blockPosition;
    glm::vec3 m_blockNormal;

    bool isSprinting = false;
    bool m_blockFound = false;
    bool m_isGrounded = true;

    bool m_isFlying = true;

    float m_speed;
    float m_defaultSpeed = 5.0f;
    float m_speedMultiplier = 1.0f;
    float m_defaultSpeedMultiplier = 1.0f;
    float m_gravity = -15.0f;
    float m_verticalVelocity = 0.0f;

    float m_height = 1.5f;
    float m_width = 0.25f;

    std::unique_ptr<Camera> m_camera;

    void updateCamera();

    void handleCollisions(float dx, float dy, float dz);

    void onPressedKey(int key, const std::function<void()>& callback);
};