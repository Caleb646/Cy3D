#include "FirstApp.h"

namespace cy3d
{
	void FirstApp::run()
	{
		while (!cyWindow.shouldClose())
		{
			glfwPollEvents();
		}
	}
}



