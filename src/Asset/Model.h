#pragma once
#include "Vulkan/XVKCommon.h"
#include "Utility/ResourceLoader.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKImage.h"
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
			tinygltf::Image glTFImage;
			VkDeviceSize bufferSize;
			unsigned char* bufferData;
		};

		struct Texture {
			int32_t imageIndex;
		};

		xvk::XVKDevice* xvk_device;
		VkQueue copyQueue;

		std::vector<AglTFModel::Node*> nodes;
		std::vector<AMaterial> materials;
		std::vector<AglTFModel::Image> images;
		std::vector<AglTFModel::Texture> textures;
		std::vector<AVertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;

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
	};
}