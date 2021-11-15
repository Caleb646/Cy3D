#pragma once

#include "CyWindow.h"

namespace cy3d
{
	class FirstApp
	{
	private:
		CyWindow cyWindow{ WindowTraits{ 800, 600, "Hello" } };

	public:
		void run();
	};

}


