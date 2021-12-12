#pragma once
#include "pch.h"

#include "platform/Vulkan/VulkanContext.h"
#include "Mesh.h"


namespace cy3d
{
	class Model
	{
	private:
		VulkanContext& _context;
		//std::vector<Texture> _loadedTextures;
		//std::vector<Mesh> _meshes;
		std::string _directory;
		std::string _path;
	public:
		Model(VulkanContext& context, const std::string& path);

		Model() = delete;
		Model(const Model& m) = delete;
		Model& operator=(const Model& m) = delete;
		

		//void drawInstanced(const Shader& shader, unsigned int amount);
		//void drawStatic(const Shader& shader);

	private:
		void loadModel(const std::string& path);
		//void loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
		uint32_t textureFromFile(const char* path, const std::string& directory, bool gamma = false);
		//void processNode(aiNode* node, const aiScene* scene);
		//Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	};
}


