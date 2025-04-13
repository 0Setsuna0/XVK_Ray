#pragma once
#include "Vulkan/XVKCommon.h"
#include "Utility/ResourceLoader.h"
#define GLM_FORCE_RIGHT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include "Vertex.h"
#include "Material.h"

namespace vkAsset
{
	class AglTFModel
	{
	public:
		struct Node;

		struct Primitive
		{
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t materialIndex;
		};

		struct Mesh
		{
			std::vector<Primitive> primitives;
		};

		struct Node
		{
			Node* parent;
			std::vector<Node*> children;
			Mesh mesh;
			glm::mat4 matrix;
			std::string name;
			bool visible = true;
			~Node()
			{
				for (auto& child : children)
				{
					delete child;
				}
			}
		};

		struct Image
		{
			VkDeviceSize bufferSize;
			uint32_t width;
			uint32_t height;
			unsigned char* bufferData;
			std::string url;
		};

		struct Texture {
			int32_t imageIndex;
		};

		std::vector<AglTFModel::Node*> nodes;
		std::vector<AglTFModel::Image> images;
		std::vector<AglTFModel::Texture> textures;

		std::vector<AVertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		std::vector<AMaterial> materials;

		AglTFModel();
		~AglTFModel();

		//----
		//glTF model loading function
		//----
		void loadImages(tinygltf::Model& input);

		void loadTextures(tinygltf::Model& input);

		void loadMaterials(tinygltf::Model& input);

		void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, AglTFModel::Node* parent,
			std::vector<uint32_t>& indexBuffer, std::vector<AVertex>& vertexBuffer);

		void loadglTFFile(std::string filePath);

		uint32_t NumVertices() const { return static_cast<uint32_t>(vertexBuffer.size()); }
		uint32_t NumIndices() const { return static_cast<uint32_t>(indexBuffer.size()); }
		uint32_t NumMaterials() const { return static_cast<uint32_t>(materials.size()); }
	};
}