#pragma once

#include "camera.h"
#include "cube.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class World;


class Player {
public:
    Player(glm::vec3 position, World* world);

    void update(float deltaTime);
    void processInput(GLFWwindow* window, float deltaTime);
    void processMouseMovement(double xpos, double ypos);

    const glm::vec3& getWorldPosition() const { return m_position; }

	glm::mat4 getView() const {
		return m_camera->getViewMatrix();
	}

	glm::mat4 getProjection() const {
		return m_camera->getProjectionMatrix();
	}

    glm::vec3 getBlockPosition() const {
        if (m_blockFound) {
            return m_blockPosition;
        }
        else {
            return glm::vec3(0, 0, 0);
        }
    }

    bool isBlockFound() const {
        return m_blockFound;
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
    bool m_isGrounded = false;

    bool m_isFlying = true;

    float m_speed;
    float m_defaultSpeed = 5.0f;
    float m_speedMultiplier = 1.0f;
    float m_defaultSpeedMultiplier = 1.0f;
    float m_gravity = -20.0f;
    float m_verticalVelocity = 0.0f;
	float m_jumpVelocity = 6.5f;

    float m_height = 1.75f;
    float m_width = 0.35f;

    std::unordered_set<BlockType> m_nonSelectableBlockTypes = { 
        BlockType::Water, 
        BlockType::None 
    };

    std::unique_ptr<Camera> m_camera;
    World* m_world;

    void updateCamera();

    void updateGrounded();

    void handleCollisions(float dx, float dy, float dz);

    void handleVerticalCollisions();

    void onPressedKey(int key, const std::function<void()>& callback);

    bool rayCast(float maxDistance, glm::vec3& outBlockPosition, glm::vec3& outNormal) const;
};