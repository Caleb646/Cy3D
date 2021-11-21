#pragma once

#include "pch.h"
#include "Fwd.hpp"

namespace cy3d
{
	struct WindowTraits
	{
		uint32_t  width;
		uint32_t  height;
		std::string windowName;

		WindowTraits(int&& w, int&& h, std::string&& name) : width(std::move(w)), height(std::move(h)), windowName(std::move(name)) {}
	};

	class VulkanWindow
	{
	private:
		//custom destructor for GLFWwindow ptr;
		struct DestroyWindow { void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); } };

		using windowPtr = std::unique_ptr<GLFWwindow, DestroyWindow>;

		bool framebufferResized{ false };
		WindowTraits windowTraits;
		windowPtr window;

	public:
		VulkanWindow(WindowTraits);
		~VulkanWindow();

		//remove copy methods
		VulkanWindow() = delete;
		VulkanWindow(const VulkanWindow&) = delete;
		VulkanWindow(VulkanWindow&&) = delete;
		VulkanWindow& operator=(const VulkanWindow&) = delete;

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		bool shouldClose() { return glfwWindowShouldClose(window.get()); }

		/**
		 * @brief Vulkan works with pixels, so the swap chain extent must be specified in pixels as well.
		 * if you are using a high DPI display (like Apple's Retina display), screen coordinates don't correspond to pixels.
		 * Instead, due to the higher pixel density, the resolution of the window in pixel will be larger than the resolution in
		 * screen coordinates. So if Vulkan doesn't fix the swap extent for us, we can't just use the original {WIDTH, HEIGHT}.
		 * Instead, we must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it against
		 * the minimum and maximum image extent.
		*/
		VkExtent2D getExtent() { return { windowTraits.width, windowTraits.height }; }
		void getWindowFrameBufferSize(int outWidth, int outHeight) { return glfwGetFramebufferSize(window.get(), &outWidth, &outHeight); }
		bool isWindowFrameBufferResized() { return framebufferResized; }
		bool isWindowMinimized() { return windowTraits.width == 0 || windowTraits.height == 0; }
		void resetWindowFrameBufferResizedFlag() { framebufferResized = false; }
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




