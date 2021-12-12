#include "pch.h"

#include "Model.h"

namespace cy3d
{
    Model::Model(VulkanContext& context, const std::string& path) : _context(context), _path(path)
    {
        loadModel(path);
    }

    void Model::loadModel(const std::string& path)
	{

		MD_ASSERT(std::filesystem::exists(path));

		Assimp::Importer importer{};
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR(importer.GetErrorString());
			return;
		}
		_directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		//processNode(scene->mRootNode, scene);
	}
	//void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	//{
 //       std::vector<Texture> textures;
 //       for (std::size_t i = 0; i < mat->GetTextureCount(type); i++)
 //       {
 //           aiString str;
 //           mat->GetTexture(type, i, &str);
 //           bool skip = false;
 //           for (std::size_t j = 0; j < _loadedTextures.size(); j++)
 //           {
 //               if (std::strcmp(_loadedTextures[j].path.data(), str.C_Str()) == 0)
 //               {
 //                   skip = true;
 //                   break;
 //               }
 //           }
 //           if (!skip)
 //           {
 //               /*Texture texture{};
 //               texture.type = typeName;
 //               texture.path = str.C_Str();
 //               texture.textureBuffer.reset(new VulkanTexture(_context, str.C_Str()));
 //               _loadedTextures.push_back(texture);*/
 //           }
 //       }
	//}

	//void Model::processNode(aiNode* node, const aiScene* scene)
	//{
	//	// process each mesh located at the current node
	//	for (std::size_t i = 0; i < node->mNumMeshes; i++)
	//	{
	//		// the node object only contains indices to index the actual objects in the scene. 
	//		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
	//		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	//		_meshes.push_back(processMesh(mesh, scene));
	//	}
	//	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	//	for (std::size_t i = 0; i < node->mNumChildren; i++)
	//	{
	//		processNode(node->mChildren[i], scene);
	//	}
	//}

 //   Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
 //   {
 //       std::vector<Vertex> vertices;
 //       std::vector<uint32_t> indices;

 //       for (std::size_t i = 0; i < mesh->mNumVertices; i++)
 //       {
 //           Vertex vertex;
 //           m3d::vec3f vector;

 //           vector.x() = mesh->mVertices[i].x;
 //           vector.y() = mesh->mVertices[i].y;
 //           vector.z() = mesh->mVertices[i].z;
 //           vertex.position = vector;

 //           // normals
 //           if (mesh->HasNormals())
 //           {
 //               vector.x() = mesh->mNormals[i].x;
 //               vector.y() = mesh->mNormals[i].y;
 //               vector.z() = mesh->mNormals[i].z;
 //               vertex.normal = vector;
 //           }

 //           if (mesh->HasTextureCoords(0))
 //           {
 //               m3d::vec2f vec;
 //               vec.x() = mesh->mTextureCoords[0][i].x;
 //               vec.y() = mesh->mTextureCoords[0][i].y;
 //               vertex.texCoords = vec;
 //           }

 //           if (mesh->HasTangentsAndBitangents())
 //           {
 //               vector.x() = mesh->mTangents[i].x;
 //               vector.y() = mesh->mTangents[i].y;
 //               vector.z() = mesh->mTangents[i].z;
 //               vertex.tangent = vector;

 //               vector.x() = mesh->mBitangents[i].x;
 //               vector.y() = mesh->mBitangents[i].y;
 //               vector.z() = mesh->mBitangents[i].z;
 //               vertex.bitTangent = vector;
 //           }

 //           else
 //           {
 //               vertex.texCoords = m3d::vec2f(0.0f, 0.0f);
 //           }

 //           vertices.push_back(vertex);
 //       }
 //       // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
 //       for (std::size_t i = 0; i < mesh->mNumFaces; i++)
 //       {
 //           aiFace face = mesh->mFaces[i];
 //           // retrieve all indices of the face and store them in the indices vector
 //           for (std::size_t j = 0; j < face.mNumIndices; j++)
 //           {
 //               indices.push_back(face.mIndices[j]);
 //           }    
 //       }

 //       aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
 //       loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
 //       // 2. specular maps
 //       loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
 //       // 3. normal maps
 //       loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
 //       // 4. height maps
 //       loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");

 //       // return a mesh object created from the extracted mesh data
 //       return Mesh{ vertices, indices };
 //   }

	//uint32_t Model::textureFromFile(const char* path, const std::string& directory, bool gamma)
	//{
	//	return uint32_t();
	//}
}
