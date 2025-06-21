#include "application.h"
#include "voxl.h"
#include <iostream>


Application::Application()
{
}


Application::~Application()
{
	if (world) {
		delete world;
		world = nullptr;
	}
	if (renderer) {
		renderer->shutdown();
		renderer.reset();
	}
	if (player) {
		delete player;
		player = nullptr;
	}
	glfwTerminate();
	std::cout << "Application shutdown successfully." << std::endl;
}

void Application::init()
{
	renderer = std::make_unique<Renderer>();
	renderer->init();

	world = new World();
	world->init();
	renderer->setWorld(world);

	player = new Player(glm::vec3(0.0f, 0.0f, 5.0f));
	world->setPlayer(player);

	// Window settings
	glfwSetWindowUserPointer(renderer->window, player);
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
