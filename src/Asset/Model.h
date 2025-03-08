#pragma once
#include "Vulkan/XVKCommon.h"
#include "Utility/ResourceLoader.h"
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

		struct Material
		{
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			uint32_t baseColorTextureIndex;
		};

		struct Image
		{
			VkDeviceSize bufferSize;
			uint32_t width;
			uint32_t height;
			unsigned char* bufferData;
		};

		struct Sampler
		{
			tinygltf::Sampler glTFSampler;
			VkSamplerAddressMode addressModeU;
			VkSamplerAddressMode addressModeV;
			VkSamplerAddressMode addressModeW;

			VkFilter minFilter;
			VkFilter maxFilter;
		};

		struct Texture {
			int32_t imageIndex;
			int32_t samplerIndex;
		};



		std::vector<AglTFModel::Node*> nodes;
		std::vector<AglTFModel::Image> images;
		std::vector<AglTFModel::Sampler> samplers;
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

		void loadSampler(tinygltf::Model& input);

		void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, AglTFModel::Node* parent,
			std::vector<uint32_t>& indexBuffer, std::vector<AVertex>& vertexBuffer);

		void loadglTFFile(std::string filePath);
	};
}