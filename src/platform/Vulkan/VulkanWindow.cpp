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
				func.second(keyboardEvent, deltaTime);
			}
		}

		for (auto& mouseEvent : mouseInputEvents)
		{
			for (auto& func : mouseInputListeners)
			{
				func.second(mouseEvent, deltaTime);
			}
		}

		keyboardInputEvents.clear();
		mouseInputEvents.clear();
	}

	VulkanWindow::listener_id VulkanWindow::registerKeyboardListener(keyboard_input_callback&& cb)
	{
		keyboardInputListeners[listenersCount] = std::move(cb);
		return listenersCount++;
	}

	VulkanWindow::listener_id VulkanWindow::registerMouseListener(mouse_input_callback&& cb)
	{
		mouseInputListeners[listenersCount] = std::move(cb);
		return listenersCount++;
	}

	void VulkanWindow::removeKeyboardListener(listener_id id)
	{
		CY_ASSERT(keyboardInputListeners.count(id) != 0);
		keyboardInputListeners.erase(id);
	}

	void VulkanWindow::removeMouseListener(listener_id id)
	{
		CY_ASSERT(mouseInputListeners.count(id) != 0);
		mouseInputListeners.erase(id);
	}

	void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		CY_ASSERT(glfwCreateWindowSurface(instance, window.get(), nullptr, surface) == VK_SUCCESS);
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

	void VulkanWindow::addKeyboardInputEvent(KeyBoardInputEvent e)
	{
		//keyboardInputEvents.push_back(std::move(e));
		keyboardInputEvents.push_back(e);
	}

	void VulkanWindow::addMouseInputEvent(MouseInputEvent e)
	{
		//mouseInputEvents.push_back(std::move(e));
		mouseInputEvents.push_back(e);
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
		cyWin->addKeyboardInputEvent(KeyBoardInputEvent{ key, scancode, action, mods });
	}

	void VulkanWindow::mouseInputCallback(window_type* window, double xpos, double ypos)
	{
		auto cyWin = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window)); 
		cyWin->addMouseInputEvent(MouseInputEvent{ xpos, ypos });
	}

	void VulkanWindow::framebufferResizeCallback(window_type* window, int width, int height)
	{
		//
		auto cyWin = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		cyWin->framebufferResized = true;
		cyWin->resetWidthHeight(width, height);
	}
}

