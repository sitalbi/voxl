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

	void setWorld(World* worldRef) {
		world = worldRef;
	}

	GLFWwindow* window;
	unsigned int m_crosshairTextureId = 0;

private:

	World* world; // Reference to the world object

	std::unique_ptr<Shader> shader;
	std::unique_ptr<Shader> highlightShader;
	std::unique_ptr<Shader> skyShader;
	std::unique_ptr<Mesh> cubeMesh;


	void render();
	void renderUI();
	void swapBuffers();
};