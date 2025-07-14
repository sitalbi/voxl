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

	// Set clear color
	glClearColor(0.1f, 0.7f, 1.0f, 1.0f);
	
	// Shader initialization
	shader = std::make_unique<Shader>(VOXL_RES_DIR "/shaders/default_vert.glsl", VOXL_RES_DIR"/shaders/default_frag.glsl");

	highlightShader = std::make_unique<Shader>(VOXL_RES_DIR "/shaders/highlight_vert.glsl", VOXL_RES_DIR "/shaders/highlight_frag.glsl");
	highlightShader->bind();

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	// Shader uniforms
	shader->bind();
	shader->setUniformMat4f("uView", world->getPlayer()->getView());
	shader->setUniformMat4f("uProjection", world->getPlayer()->getProjection());


	// Draw the chunk
	for (auto& chunk : world->getRenderList())
	{
		if (chunk)
		{
			shader->setUniformMat4f("uModel", glm::translate(glm::mat4(1.0f), chunk->getWorldPosition()));
			chunk->draw();
		}
	}


	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	// Draw transparent chunks
	for (auto& chunk : world->getRenderList())
	{
		if (chunk)
		{
			shader->setUniformMat4f("uModel", glm::translate(glm::mat4(1.0f), chunk->getWorldPosition()));

			chunk->drawTransparent();
		}
	}
	glDisable(GL_BLEND);

	// Block highlight
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
		highlightShader->setUniformMat4f("uModel", glm::scale(glm::translate(glm::mat4(1.0f), world->getPlayer()->getBlockPosition()), glm::vec3(1.05f, 1.05f, 1.05f)));
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
