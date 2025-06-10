#include "player.h"
#include "voxl.h"
#include <glm/gtc/matrix_transform.hpp> 

Player::Player(glm::vec3 position) : m_position(position)
{
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
	m_position.y += m_velocity.y * deltaTime;
	if (!m_isFlying) handleCollisions(0.0f, m_velocity.y, 0.0f);
	m_position.z += m_velocity.z * deltaTime;
	if (!m_isFlying) handleCollisions(0.0f, 0.0f, m_velocity.z);

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
            m_verticalVelocity = 6.5f;
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

void Player::handleCollisions(float dx, float dy, float dz)
{
    //TODO
}

void Player::onPressedKey(int key, const std::function<void()>& callback)
{
    bool isPressed = glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS;
    if (isPressed && !m_keyStates[key]) {
        callback();
    }
    m_keyStates[key] = isPressed;
}