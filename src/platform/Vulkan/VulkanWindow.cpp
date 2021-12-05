#include "pch.h"

#include "VulkanWindow.h"

#include <Logi/Logi.h>


namespace cy3d
{
	VulkanWindow::VulkanWindow(WindowTraits wts) : windowTraits(wts)
	{ 
		init();
	}

	VulkanWindow::~VulkanWindow()
	{
		glfwTerminate();
	}

	void VulkanWindow::processInput(double deltaTime)
	{
		glfwPollEvents();

		//process events all at once
		for (auto& keyboardEvent : keyboardInputEvents)
		{
			for (auto& func : keyboardInputListeners)
			{
				func(keyboardEvent, deltaTime);
			}
		}

		for (auto& mouseEvent : mouseInputEvents)
		{
			for (auto& func : mouseInputListeners)
			{
				func(mouseEvent, deltaTime);
			}
		}

		keyboardInputEvents.clear();
		mouseInputEvents.clear();
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

	void VulkanWindow::init()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); //set resize window to false.
		window = std::move(windowPtr(glfwCreateWindow(windowTraits.width, windowTraits.height, windowTraits.windowName.c_str(), nullptr, nullptr)));

		//GLFW function that allows you to store an arbitrary pointer inside of it:
		glfwSetWindowUserPointer(window.get(), this);

		//set keyboard input callback
		glfwSetKeyCallback(window.get(), keyboardInputCallback);
		//set mouse input callback
		glfwSetCursorPosCallback(window.get(), mouseInputCallback);
		//set resize callback
		glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
	}

	void VulkanWindow::addKeyboardInputEvent(KeyBoardInputEvent&& e)
	{
		keyboardInputEvents.push_back(std::move(e));
	}

	void VulkanWindow::addMouseInputEvent(MouseInputEvent&& e)
	{
		mouseInputEvents.push_back(std::move(e));
	}

	/**
	* 
	* 
	* PRIVATE STATIC METHODS
	* 
	* 
	*/
	void VulkanWindow::keyboardInputCallback(window_type* window, int key, int scancode, int action, int mods)
	{
		auto cyWin = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		cyWin->addKeyboardInputEvent(KeyBoardInputEvent{ cyWin->getNativeWindow(), key, scancode, action, mods });
	}

	void VulkanWindow::mouseInputCallback(window_type* window, double xpos, double ypos)
	{
		auto cyWin = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		cyWin->addMouseInputEvent(MouseInputEvent{ cyWin->getNativeWindow(), xpos, ypos });
	}

	void VulkanWindow::framebufferResizeCallback(window_type* window, int width, int height)
	{
		//
		auto cyWin = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		cyWin->framebufferResized = true;
		cyWin->resetWidthHeight(width, height);
	}
}

