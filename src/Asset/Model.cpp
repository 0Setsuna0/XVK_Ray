#include "Model.h"
#include <glm/gtc/type_ptr.hpp>

namespace vkAsset
{
	AglTFModel::AglTFModel()
	{

	}

	AglTFModel::~AglTFModel()
	{
		for (auto node : nodes)
		{
			delete node;
		}
	}

	void AglTFModel::loadImages(tinygltf::Model& input)
	{
		images.resize(input.images.size());
		for (int i = 0; i < input.images.size(); i++)
		{
			tinygltf::Image& glTFImage = input.images[i];
			images[i].width = glTFImage.width;
			images[i].height = glTFImage.height;

			unsigned char* buffer = nullptr;
			VkDeviceSize bufferSize = 0;
			
			if (glTFImage.component == 3)
			{
				bufferSize = glTFImage.width * glTFImage.height * 4;
				buffer = new unsigned char[bufferSize];
				unsigned char* rgba = buffer;
				unsigned char* rgb = &glTFImage.image[0];
				for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) 
				{
					memcpy(rgba, rgb, sizeof(unsigned char) * 3);
					rgba += 4;
					rgb += 3;
				}
			}
			else
			{
				buffer = &glTFImage.image[0];
				bufferSize = glTFImage.image.size();
			}

			images[i].bufferData = buffer;
		}
	}

	void AglTFModel::loadTextures(tinygltf::Model& input)
	{
		textures.resize(input.textures.size());
		for (int i = 0; i < input.textures.size(); i++)
		{
			textures[i].imageIndex = input.textures[i].source;
			textures[i].samplerIndex = input.textures[i].sampler;
		}
	}

	void AglTFModel::loadSampler(tinygltf::Model& input)
	{
		samplers.resize(input.samplers.size());
		for (int i = 0; i < input.samplers.size(); i++)
		{
			
		}
	}

	void AglTFModel::loadMaterials(tinygltf::Model& input)
	{
		materials.resize(input.materials.size());
		for (int i = 0; i < input.materials.size(); i++)
		{
			tinygltf::Material glTFMaterial = input.materials[i];
			//// Get the base color factor
			//if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
			//	materials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
			//}
			//// Get base color texture index
			//if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
			//	materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
			//}
		}
	}

	void AglTFModel::loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, AglTFModel::Node* parent,
		std::vector<uint32_t>& indexBuffer, std::vector<AVertex>& vertexBuffer)
	{
		AglTFModel::Node* node = new AglTFModel::Node{};
		node->matrix = glm::mat4(1.0f);
		node->parent = parent;

		if (inputNode.translation.size() == 3) 
		{
			node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}
		if (inputNode.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->matrix *= glm::mat4(q);
		}
		if (inputNode.scale.size() == 3)
		{
			node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}
		if (inputNode.matrix.size() == 16) 
		{
			node->matrix = glm::make_mat4x4(inputNode.matrix.data());
		};

		// Load node's children
		if (inputNode.children.size() > 0) 
		{
			for (size_t i = 0; i < inputNode.children.size(); i++) 
			{
				loadNode(input.nodes[inputNode.children[i]], input, node, indexBuffer, vertexBuffer);
			}
		}
	}

	void AglTFModel::loadglTFFile(std::string filePath)
	{
		tinygltf::Model glTFInput;
		tinygltf::TinyGLTF glTFContext;
		std::string error, warning;

		bool fileLoaded = glTFContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filePath);

		if (fileLoaded)
		{
			loadImages(glTFInput);
			loadTextures(glTFInput);
			loadMaterials(glTFInput);
			const tinygltf::Scene& scene = glTFInput.scenes[0];
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
				loadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
			}
		}
		else
		{
			throw std::runtime_error("Fail to load glTF file");
		}
	}
}