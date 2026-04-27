#include <iostream>         
#include <cstdlib>          

#include <GL/glew.h>        
#include "GLFW/glfw3.h"     

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

namespace
{
	const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones";

	GLFWwindow* g_Window = nullptr;
	SceneManager* g_SceneManager = nullptr;
	ShaderManager* g_ShaderManager = nullptr;
	ViewManager* g_ViewManager = nullptr;
}

bool InitializeGLFW();
bool InitializeGLEW();

int main(int argc, char* argv[])
{
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}

	g_ShaderManager = new ShaderManager();
	g_ViewManager = new ViewManager(g_ShaderManager);

	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	g_ShaderManager->LoadShaders(
		"../../Utilities/shaders/vertexShader.glsl",
		"../../Utilities/shaders/fragmentShader.glsl");
	g_ShaderManager->use();

	g_SceneManager = new SceneManager(g_ShaderManager);

	// --- FIX: CONNECT THE CAMERA ---
	// We get the camera object from the ViewManager and give it to the SceneManager
	// This allows the SceneManager to see where the user is looking.
	if (g_ViewManager != nullptr && g_SceneManager != nullptr)
	{
		g_SceneManager->SetCamera(g_ViewManager->GetCamera());
	}

	g_SceneManager->PrepareScene();

	while (!glfwWindowShouldClose(g_Window))
	{
		glEnable(GL_DEPTH_TEST);

		// FIX: Change clear color to dark gray so you can see if 
		// black objects are actually rendering against the background.
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_ViewManager->PrepareSceneView();

		g_SceneManager->RenderScene();

		glfwSwapBuffers(g_Window);
		glfwPollEvents();
	}

	if (NULL != g_SceneManager)
	{
		delete g_SceneManager;
		g_SceneManager = NULL;
	}
	if (NULL != g_ViewManager)
	{
		delete g_ViewManager;
		g_ViewManager = NULL;
	}
	if (NULL != g_ShaderManager)
	{
		delete g_ShaderManager;
		g_ShaderManager = NULL;
	}

	exit(EXIT_SUCCESS);
}

bool InitializeGLFW()
{
	glfwInit();
#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// Using 4.4+ is standard for modern Windows/Linux systems
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	return(true);
}

bool InitializeGLEW()
{
	GLenum GLEWInitResult = GLEW_OK;
	GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}
	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

	return(true);
}