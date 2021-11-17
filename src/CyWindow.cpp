#include "CyWindow.h"


#include <stdexcept>

namespace cy3d
{
	CyWindow::CyWindow(WindowTraits wts) : windowTraits(wts) { initWindow(); }

	CyWindow::~CyWindow()
	{
		glfwTerminate();
	}

	void CyWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window.get(), nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void CyWindow::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //set resize window to false.
		window = std::move(windowPtr(glfwCreateWindow(windowTraits.width, windowTraits.height, windowTraits.windowName.c_str(), nullptr, nullptr)));
	}
}

