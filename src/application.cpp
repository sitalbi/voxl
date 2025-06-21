#include "application.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include "voxl.h"


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

	player = new Player(glm::vec3(32.0f, 68.0f, 32.0f));
	world->setPlayer(player);

	// Window settings
	glfwSetWindowUserPointer(renderer->window, player);
	glfwSetCursorPosCallback(renderer->window, mouseCallback);
	glfwSetInputMode(renderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	initUI();
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

		updateUI();
		world->update(deltaTime);
		player->update(deltaTime);
		renderer->update(deltaTime);

		glfwPollEvents();
	}
}

void Application::shutdown()
{
	renderer->shutdown();
	glfwTerminate();
}


void Application::initUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(renderer->window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

}

void Application::updateUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	if (ImGui::Begin("Info", nullptr, window_flags)) {
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

		if (player) {
			const glm::vec3& pos = player->getPosition();
			ImGui::Text("Player World Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
			ImGui::Text("Player Chunk Position: %d, %d, %d", (int)(pos.x / Chunk::CHUNK_SIZE), 0, (int)(pos.z / Chunk::CHUNK_SIZE));
		}
	}
	ImGui::End();
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