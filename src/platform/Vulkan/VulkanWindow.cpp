#include "pch.h"

#include "VulkanWindow.h"

#include <Logi/Logi.h>


namespace cy3d
{
	VulkanWindow::VulkanWindow(WindowTraits wts) : windowTraits(wts) { initWindow(); }

	VulkanWindow::~VulkanWindow()
	{
		glfwTerminate();
	}

	void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		ASSERT_ERROR(DEFAULT_LOGGABLE, glfwCreateWindowSurface(instance, window.get(), nullptr, surface) == VK_SUCCESS, "Failed to create window surface.");
	}

	/**
	 * @brief Will block until the window is no longer minimized using glfwWaitEvents().
	*/
	void VulkanWindow::blockWhileWindowMinimized()
	{
		while (isWindowMinimized()) glfwWaitEvents();
	}

	void VulkanWindow::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); //set resize window to false.
		window = std::move(windowPtr(glfwCreateWindow(windowTraits.width, windowTraits.height, windowTraits.windowName.c_str(), nullptr, nullptr)));

		//GLFW function that allows you to store an arbitrary pointer inside of it:
		glfwSetWindowUserPointer(window.get(), this);
		//set resize callback
		glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
	}

	/**
	 * PRIVATE STATIC METHODS
	*/
	void VulkanWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		//
		auto cyWin = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		cyWin->framebufferResized = true;
		cyWin->resetWidthHeight(width, height);
	}
}

