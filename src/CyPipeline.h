#pragma once

#include <string>
#include <vector>

namespace cy3d
{
	class CyPipeline
	{
	public:
		CyPipeline(const std::string& vertFilepath, const std::string& fragFilepath);

	private:
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);

		/*
		* STATIC METHODS
		*/
		static std::vector<char> readFile(const std::string& filename);
	};
}


