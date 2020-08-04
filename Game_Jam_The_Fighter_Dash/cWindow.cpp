#include "cWindow.h"
#include <iostream>
#include "GFLW_callbacks.h"
#include "cCamera.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

cWindow::cWindow()
{
	this->windowWidth = 1280;
	this->windowHeight = 720;
	//this->windowWidth = 1920;
	//this->windowHeight = 1080;
	this->windowTitle = "Game Jam The Fighter Dash";
	std::cout << "A cWindow() is created." << std::endl;
	return;
}

cWindow::~cWindow()
{
	std::cout << "A cWindow() is destroyed." << std::endl;
	return;
}

bool cWindow::createWindow()
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(1);

	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	//glEnable(GL_FRAMEBUFFER_SRGB);
	
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	return true;
}
