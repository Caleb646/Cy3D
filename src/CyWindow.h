#pragma once

#include "pch.h"


namespace cy3d
{
	struct WindowTraits
	{
		const int width;
		const int height;
		std::string windowName;

		WindowTraits(int&& w, int&& h, std::string&& name) : width(std::move(w)), height(std::move(h)), windowName(std::move(name)) {}
	};

	class CyWindow
	{
	private:
		struct DestroyWindow //custom destructor for GLFWwindow ptr;
		{
			void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); }
		};

		using windowPtr = std::unique_ptr<GLFWwindow, DestroyWindow>;

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
		VkExtent2D getExtent() { return { static_cast<uint32_t>(windowTraits.width), static_cast<uint32_t>(windowTraits.height) }; }

	private:
		void initWindow();
	};
};




