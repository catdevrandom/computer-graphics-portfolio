///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ShaderManager.h"
#include "camera.h"      // Ensure this file exists and defines the Camera class
#include "GLFW/glfw3.h" 

/***********************************************************
 * ViewManager
 *
 * This class manages the window creation, camera input,
 * and projection/view matrix setup.
 ***********************************************************/
class ViewManager
{
public:
	ViewManager(ShaderManager* pShaderManager);
	~ViewManager();

	// Callback functions for GLFW
	static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);
	static void Mouse_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset);

	// --- FIX: Public method to allow other classes to access the camera ---
	Camera* GetCamera() { return m_pCamera; }

	// Window and Scene setup
	GLFWwindow* CreateDisplayWindow(const char* windowTitle);
	void PrepareSceneView();

private:
	ShaderManager* m_pShaderManager;
	GLFWwindow* m_pWindow;

	// --- FIX: Added the Camera pointer member variable ---
	Camera* m_pCamera;

	void ProcessKeyboardEvents();
};