#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

namespace cy3d
{
	struct WindowTraits
	{
		const int width;
		const int height;
		std::string windowName;

		WindowTraits(int&& w, int&& h, std::string&& name) : width(std::move(w)), height(std::move(h)), windowName(std::move(name)) {}
	};

	namespace _internal
	{
		//specify destructor for std::unique_ptr for glfw window.
		struct DestroyWindow
		{
			void operator()(GLFWwindow* ptr)
			{
				glfwDestroyWindow(ptr);
			}
		};

		typedef std::unique_ptr<GLFWwindow, DestroyWindow> windowPtr;
	}



	class CyWindow
	{
	private:
		WindowTraits windowTraits;
		_internal::windowPtr window;

	public:
		CyWindow(WindowTraits);
		~CyWindow();

		//remove copy methods
		CyWindow(const CyWindow&) = delete;
		CyWindow& operator=(const CyWindow&) = delete;

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		bool shouldClose() { return glfwWindowShouldClose(window.get()); }

	private:
		void initWindow();
	};
};




