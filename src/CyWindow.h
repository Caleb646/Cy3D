#pragma once

#include "pch.h"


namespace cy3d
{
	struct WindowTraits
	{
		uint32_t  width;
		uint32_t  height;
		std::string windowName;

		WindowTraits(int&& w, int&& h, std::string&& name) : width(std::move(w)), height(std::move(h)), windowName(std::move(name)) {}
	};

	class CyWindow
	{
	private:
		//custom destructor for GLFWwindow ptr;
		struct DestroyWindow { void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); } };

		using windowPtr = std::unique_ptr<GLFWwindow, DestroyWindow>;

		bool framebufferResized{ false };
		WindowTraits windowTraits;
		windowPtr window;

	public:
		CyWindow(WindowTraits);
		~CyWindow();

		//remove copy methods
		CyWindow(const CyWindow&) = delete;
		CyWindow& operator=(const CyWindow&) = delete;

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		bool shouldClose() { return glfwWindowShouldClose(window.get()); }
		VkExtent2D getExtent() { return { windowTraits.width, windowTraits.height }; }
		void getWindowFrameBufferSize(int outWidth, int outHeight) { return glfwGetFramebufferSize(window.get(), &outWidth, &outHeight); }
		bool isWindowFrameBufferResized() { return framebufferResized; }
		bool isWindowMinimized() { return windowTraits.width == 0 || windowTraits.height == 0; }
		void resetWindowFrameBufferResized() { framebufferResized = false; }
		void blockWhileWindowMinimized();


	private:
		void resetWidthHeight(uint32_t w, uint32_t h) { windowTraits.width = w; windowTraits.height = h; }
		void initWindow();

		/**
		 * PRIVATE STATIC METHODS
		*/
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
};




