#include "CyWindow.h"

namespace cy3d
{
	CyWindow::CyWindow(WindowTraits wts) : windowTraits(wts) { initWindow(); }

	CyWindow::~CyWindow()
	{
		glfwTerminate();
	}

	void CyWindow::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //set resize window to false.
		window = std::move(_internal::windowPtr(glfwCreateWindow(windowTraits.width, windowTraits.height, windowTraits.windowName.c_str(), nullptr, nullptr)));
		//window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}

}

