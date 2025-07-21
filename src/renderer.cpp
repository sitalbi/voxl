#include "glad/glad.h"
#include "renderer.h"
#include <iostream>
#include "voxl.h"
#include <texture.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>


Renderer::Renderer() : window(nullptr)
{
}

Renderer::Renderer(GLFWwindow* windowContext) : window(windowContext)
{
}

Renderer::~Renderer()
{
	shutdown();
}

bool Renderer::init()
{
	if (m_isInitialized)
	{
		return true;
	}

	// Set the OpenGL version and profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   
	// Initialize GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Voxl", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		std::cerr << "Failed to create GLFW window" << std::endl;
	}
	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return false;
	}

	// Load crosshair texture
	m_crosshairTextureId = Texture::loadFromFile(VOXL_RES_DIR "/textures/crosshair.png");

	// Cube mesh
	cubeMesh = std::make_unique<Mesh>();
	cubeMesh->createCube();

	
	
	// Shader initialization
	shader = std::make_unique<Shader>(VOXL_RES_DIR "/shaders/default_vert.glsl", VOXL_RES_DIR"/shaders/default_frag.glsl");

	highlightShader = std::make_unique<Shader>(VOXL_RES_DIR "/shaders/highlight_vert.glsl", VOXL_RES_DIR "/shaders/highlight_frag.glsl");

	skyShader = std::make_unique<Shader>(VOXL_RES_DIR "/shaders/sky_vert.glsl", VOXL_RES_DIR "/shaders/sky_frag.glsl");

	// Texture atlas initialization
	Texture textureAtlas;
	bool textureLoaded = textureAtlas.loadTextureArrayFromFile(VOXL_RES_DIR "/textures/default_texture.png", Atlas::COLS, Atlas::ROWS);
	if (!textureLoaded) {
		std::cerr << "Failed to load texture atlas" << std::endl;
		return false;
	}
	
	// Set the texture uniform in the shader
	textureAtlas.bind(1);
	shader->bind();
	shader->setUniform1i("uTextureArray", 1);
	shader->setUniform1f("uFogStart", ((World::CHUNK_LOAD_RADIUS) * Chunk::CHUNK_SIZE)/2);
	shader->setUniform1f("uFogEnd", ((World::CHUNK_LOAD_RADIUS) * Chunk::CHUNK_SIZE)/1.5f);

    
	// Backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

    // Enable depth 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable antialiasing
    glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);


	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);


	m_isInitialized = true;

    return true;
}

void Renderer::update(float deltaTime)
{
	render();
	renderUI();
	swapBuffers();
}

void Renderer::shutdown()
{
	shader->unbind();
	glfwDestroyWindow(window);
}

void Renderer::render()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	// 1 - Draw fullscreen sky quad
	glDisable(GL_DEPTH_TEST);
	skyShader->bind();
	skyShader->setUniform3f("uSkyColor", world->getSkyColor().x, world->getSkyColor().y, world->getSkyColor().z);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glEnable(GL_DEPTH_TEST);

	// Default shader uniforms
	shader->bind();
	shader->setUniformMat4f("uView", world->getPlayer()->getView());
	shader->setUniformMat4f("uProjection", world->getPlayer()->getProjection());
	shader->setUniform1f("uLightIntensity", world->getLightIntensity());
	shader->setUniform3f("uFogColor", world->getSkyColor().x, world->getSkyColor().y, world->getSkyColor().z);


	// 2 - Draw the world chunks
	// Opaque chunks
	for (auto& chunk : world->getRenderList())
	{
		if (chunk)
		{
			shader->setUniformMat4f("uModel", glm::translate(glm::mat4(1.0f), chunk->getWorldPosition()));
			chunk->draw();
		}
	}


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Transparent chunks
	for (auto& chunk : world->getRenderList())
	{
		if (chunk)
		{
			shader->setUniformMat4f("uModel", glm::translate(glm::mat4(1.0f), chunk->getWorldPosition()));

			chunk->drawTransparent();
		}
	}
	glDisable(GL_BLEND);

	// 3- Draw the selected block highlight
	if (world->getPlayer()->isBlockFound()) 
	{
		glDisable(GL_CULL_FACE);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		// Enable writing to the stencil buffer
		glStencilMask(0xFF);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		shader->bind();
		shader->setUniformMat4f("uModel", glm::scale(glm::translate(glm::mat4(1.0f), world->getPlayer()->getBlockPosition()), glm::vec3(1.01f, 1.01f, 1.01f)));
		shader->setUniformBool("uColorBlock", true);
		cubeMesh->draw();
		shader->setUniformBool("uColorBlock", false);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDepthMask(GL_FALSE);
		glLineWidth(3);

		highlightShader->bind();
		highlightShader->setUniformMat4f("uView", world->getPlayer()->getView());
		highlightShader->setUniformMat4f("uProjection", world->getPlayer()->getProjection());
		highlightShader->setUniformMat4f("uModel", glm::scale(glm::translate(glm::mat4(1.0f), world->getPlayer()->getBlockPosition()), glm::vec3(1.025f, 1.025f, 1.025f)));
		cubeMesh->draw();

		glDisable(GL_POLYGON_OFFSET_FILL);

		// Re-enable depth writing and stencil writing
		glDepthMask(GL_TRUE);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	// Player wireframe mode
	if (world->getPlayer()->wireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}

	
}

void Renderer::renderUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::swapBuffers()
{
	glfwSwapBuffers(window);
}
