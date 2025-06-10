#pragma once

#include "subsystem.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <world.h>

class Renderer : public ISubsystem
{
public:
	Renderer();
	Renderer(GLFWwindow* windowContext);
	~Renderer() override;


	virtual bool  init() override;
	virtual void  update(float deltaTime) override;
	virtual void  shutdown() override;

	void setWorld(std::shared_ptr<World> worldRef) {
		world = worldRef;
	}

	GLFWwindow* window;

private:

	std::shared_ptr<World> world; // Reference to the world object

	std::unique_ptr<Shader> shader;
};