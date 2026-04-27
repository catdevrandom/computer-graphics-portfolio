///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////
#include "ViewManager.h"
#include <iostream>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

namespace
{
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// Static instance pointer so GLFW callbacks can access the camera
	ViewManager* g_CurrentInstance = nullptr;

	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *  Constructor - initializes the camera with a default
 *  position looking down at the scene.
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;

	// Create the camera object
	m_pCamera = new Camera();
	g_CurrentInstance = this;

	// Initial camera setup - positioned above and back, looking down
	m_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	m_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	m_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_pCamera->Zoom = 45.0f;
	m_pCamera->MovementSpeed = 5.0f;
}

/***********************************************************
 *  ~ViewManager()
 *  Destructor - cleans up allocated camera memory.
 ***********************************************************/
ViewManager::~ViewManager()
{
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *  Creates the GLFW window, sets up mouse input callbacks,
 *  and enables alpha blending for transparency.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// Capture the mouse cursor for camera control
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// Enable blending for semi-transparent objects
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;
	return window;
}

/***********************************************************
 *  Mouse_Position_Callback()
 *  Handles mouse movement to rotate the camera view.
 *  Calculates the offset from the last known position
 *  and passes it to the camera for pitch/yaw updates.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// On first mouse input, initialize position to avoid a large jump
	if (gFirstMouse)
	{
		gLastX = (float)xMousePos;
		gLastY = (float)yMousePos;
		gFirstMouse = false;
	}

	// Calculate how far the mouse moved since last frame
	float xoffset = (float)xMousePos - gLastX;
	float yoffset = gLastY - (float)yMousePos;  // FIX: was xMousePos, must be yMousePos

	// Store current position for next frame's calculation
	gLastX = (float)xMousePos;
	gLastY = (float)yMousePos;

	// Send the movement deltas to the camera
	if (g_CurrentInstance && g_CurrentInstance->m_pCamera)
		g_CurrentInstance->m_pCamera->ProcessMouseMovement(xoffset, yoffset);
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *  Handles scroll wheel input to adjust camera movement
 *  speed. Scrolling up increases speed, scrolling down
 *  decreases it, with a minimum of 1.0.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (g_CurrentInstance && g_CurrentInstance->m_pCamera)
	{
		g_CurrentInstance->m_pCamera->MovementSpeed += (float)yoffset * 2.0f;
		if (g_CurrentInstance->m_pCamera->MovementSpeed < 1.0f)
			g_CurrentInstance->m_pCamera->MovementSpeed = 1.0f;
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *  Polls keyboard state each frame for camera controls:
 *    W/S - Forward/Backward
 *    A/D - Strafe Left/Right
 *    Q/E - Move Up/Down
 *    P   - Switch to Perspective projection
 *    O   - Switch to Orthographic projection
 *    ESC - Close the window
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_pWindow, true);

	// Horizontal and depth movement
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS) m_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS) m_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS) m_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS) m_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);

	// Vertical movement
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		m_pCamera->Position += m_pCamera->Up * (m_pCamera->MovementSpeed * gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		m_pCamera->Position -= m_pCamera->Up * (m_pCamera->MovementSpeed * gDeltaTime);

	// Projection toggle
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS) bOrthographicProjection = false;
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS) bOrthographicProjection = true;
}

/***********************************************************
 *  PrepareSceneView()
 *  Called each frame to update timing, process input, and
 *  send the current View and Projection matrices to the
 *  shader. Supports both perspective and orthographic modes.
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// Calculate delta time for smooth movement at any framerate
	float currentFrame = (float)glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// Process all keyboard input
	ProcessKeyboardEvents();

	// Get the current view matrix from the camera
	view = m_pCamera->GetViewMatrix();

	// Choose projection type based on user toggle (P/O keys)
	if (!bOrthographicProjection)
	{
		projection = glm::perspective(
			glm::radians(m_pCamera->Zoom),
			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT,
			0.1f, 100.0f);
	}
	else
	{
		float scale = 5.0f;
		float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
		projection = glm::ortho(
			-scale * aspect, scale * aspect,
			-scale, scale,
			0.1f, 100.0f);
	}

	// Pass matrices and camera position to the shader
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", m_pCamera->Position);
	}
}
