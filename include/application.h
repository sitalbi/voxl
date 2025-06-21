#pragma once

#include "renderer.h"
#include "world.h"
#include <player.h>
#include <memory>	

class Application 
{
public:
	Application();
	~Application();

	void init();
	void run();
	void shutdown();

private:

	std::unique_ptr<Renderer> renderer;
	World* world;
	Player* player;

	float deltaTime = 0.0f; // Time between current frame and last frame	
	double lastTime = 0.0f;
	double currentTime = 0.0f;

	void initUI();
	void updateUI();

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	void mouseCallbackInstance(double xpos, double ypos);

};