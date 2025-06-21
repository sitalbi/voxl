#include "glad/glad.h"
#include "renderer.h"
#include <iostream>
#include "voxl.h"
#include <texture.h>


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

	glClearColor(0.1f, 0.7f, 1.0f, 1.0f);
	
	// Shader initialization
	shader = std::make_unique<Shader>(VOXL_RES_DIR "/shaders/default_vert.glsl", VOXL_RES_DIR"/shaders/default_frag.glsl");
	shader->bind();
	shader->setUniformMat4f("uModel", glm::mat4(1.0f));

	// Texture atlas initialization
	Texture textureAtlas;
	bool textureLoaded = textureAtlas.loadTextureArrayFromFile(
		VOXL_RES_DIR "/textures/default_texture.png",
		Atlas::COLS, Atlas::ROWS
	);
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

	m_isInitialized = true;

    return true;
}

void Renderer::update(float deltaTime)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Shader uniforms
	shader->bind();
	shader->setUniformMat4f("uView", world->getPlayer()->getView());
	shader->setUniformMat4f("uProjection", world->getPlayer()->getProjection());

	// Draw the chunk
	for (auto& chunkData : world->getChunks())
	{
		auto& chunk = chunkData.second;
		if (chunk)
		{
			shader->setUniformMat4f("uModel", glm::translate(glm::mat4(1.0f), chunk->getPosition()));
			chunk->draw();
		}
	}
	// Draw the single chunk
	/*if (world->getChunk())
	{
		world->getChunk()->draw();
	}*/

	// Player wireframe mode
	if (world->getPlayer()->wireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}

    glfwSwapBuffers(window);

    glfwPollEvents();
}

void Renderer::shutdown()
{
	shader->unbind();
	glfwDestroyWindow(window);
}