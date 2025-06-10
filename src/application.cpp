#include "application.h"
#include "voxl.h"
#include <iostream>


Application::Application()
{
}


Application::~Application()
{
}

void Application::init()
{
	renderer = std::make_unique<Renderer>();
	renderer->init();

	world = std::make_unique<World>();
	world->init();
	renderer->setWorld(world);

	player = std::make_shared<Player>(glm::vec3(0.0f, 0.0f, 5.0f));
	world->setPlayer(player);

	// Window settings
	glfwSetWindowUserPointer(renderer->window, player.get());
	glfwSetCursorPosCallback(renderer->window, mouseCallback);
	glfwSetInputMode(renderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Application::run()
{
	lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(renderer->window))
	{
		// Calculate delta time
		currentTime = glfwGetTime();
		deltaTime = static_cast<float>(currentTime - lastTime);
		lastTime = currentTime;

		renderer->update(deltaTime);
		world->update(deltaTime);
		player->update(deltaTime);
	}
}

void Application::shutdown()
{
	renderer->shutdown();
	glfwTerminate();
}

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Player* player = static_cast<Player*>(glfwGetWindowUserPointer(window));
	if (player) {
		player->processMouseMovement(xpos, ypos);
	}
	else {
		std::cerr << "Failed to retrieve player instance from window user pointer." << std::endl;
	}
}
