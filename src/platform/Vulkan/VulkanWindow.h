#pragma once
#include "pch.h"

#include "Vulkan.h"
#include "Fwd.hpp"

namespace cy3d
{
	struct WindowTraits
	{
		uint32_t  width;
		uint32_t  height;
		std::string windowName;

		WindowTraits(int w, int h, std::string&& name) : width(w), height(h), windowName(std::move(name)) {}
	};

	struct KeyBoardInputEvent
	{
		//VulkanWindow::window_type* window;
		int key;
		int scancode; 
		int action;
		int mods;

		//KeyBoardInputEvent(VulkanWindow::window_type* w, int k, int s, int a, int m) :
		//	window(w), key(k), scancode(s), action(a), mods(m) {}
	};

	struct MouseInputEvent
	{
		//VulkanWindow::window_type* window;
		double xpos;
		double ypos;

		//MouseInputEvent(VulkanWindow::window_type* w, double x, double y) :
		//	window(w), xpos(x), ypos(y) {}
	};

	static uint32_t listenersCount = 0;

	class VulkanWindow
	{

	public:
		using window_type = GLFWwindow;
		//using keyboard_input_callback = void(*)(KeyBoardInputEvent&, double deltaTime);
		//using mouse_input_callback = void(*)(MouseInputEvent&, double deltaTime);		
		using keyboard_input_callback = std::function<void(KeyBoardInputEvent&, double deltaTime)>;
		using mouse_input_callback = std::function<void(MouseInputEvent&, double deltaTime)>;

		using listener_id = uint32_t;
		template<typename T>
		using listener_container = std::unordered_map<listener_id, T>;

	private:
		//custom destructor for GLFWwindow ptr;
		struct DestroyWindow { void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); } };

		using windowPtr = std::unique_ptr<window_type, DestroyWindow>;

		bool framebufferResized{ false };
		WindowTraits windowTraits;
		windowPtr window;

		listener_container<keyboard_input_callback> keyboardInputListeners;
		std::vector<KeyBoardInputEvent> keyboardInputEvents;
		listener_container<mouse_input_callback> mouseInputListeners;
		std::vector<MouseInputEvent> mouseInputEvents;

	public:
		VulkanWindow(WindowTraits);
		~VulkanWindow();

		//remove copy methods
		VulkanWindow() = delete;
		VulkanWindow(const VulkanWindow&) = delete;
		VulkanWindow(VulkanWindow&&) = delete;
		VulkanWindow& operator=(const VulkanWindow&) = delete;

		void processInput(double deltaTime);
		listener_id registerKeyboardListener(keyboard_input_callback&& cb);
		listener_id registerMouseListener(mouse_input_callback&& cb);
		void removeKeyboardListener(listener_id id);
		void removeMouseListener(listener_id id);

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
		GLFWwindow* getNativeWindow() { return window.get(); }
		bool isWindowFrameBufferResized() { return framebufferResized; }
		bool isWindowMinimized() { return windowTraits.width == 0 || windowTraits.height == 0; }
		void resetWindowFrameBufferResizedFlag() { framebufferResized = false; }
		void blockWhileWindowMinimized();


	private:
		void resetWidthHeight(uint32_t w, uint32_t h) { windowTraits.width = w; windowTraits.height = h; }
		void init();
		void addKeyboardInputEvent(KeyBoardInputEvent e);
		void addMouseInputEvent(MouseInputEvent e);

		/**
		 * PRIVATE STATIC METHODS
		*/	
		static void keyboardInputCallback(window_type* window, int key, int scancode, int action, int mods);
		static void mouseInputCallback(window_type* window, double xpos, double ypos);
		static void framebufferResizeCallback(window_type* window, int width, int height);
	};
};




