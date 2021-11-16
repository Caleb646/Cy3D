#pragma once

#include "CyPipeline.h"
#include "CyWindow.h"

namespace cy3d
{
	class FirstApp
	{
	private:
		CyWindow cyWindow{ WindowTraits{ 800, 600, "Hello" } };
		CyPipeline cyPipeline{ "resources\shaders\SimpleShader.vert", "resources\shaders\SimpleShader.frag" };

	public:
		void run();
	};

}


