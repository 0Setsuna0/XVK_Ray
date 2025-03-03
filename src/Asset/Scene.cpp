#include "Scene.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKImage.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Vulkan/XVKImageView.h"
#include "Vulkan/XVKSampler.h"
#include "Utility/BufferUtil.h"
#include "Texture.h"
#include "TextureImage.h"
#include "Vertex.h"
#include "Material.h"

namespace vkAsset
{
	AScene::AScene(xvk::XVKCommandPool& commandPool, std::vector<AglTFModel>&& models, std::vector<AVulkanTexture>&& textures)
		:models(std::move(models)), textures(std::move(textures))
	{
		std::vector<AVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<AMaterial> materials;
		std::vector<VkAabbPositionsKHR> aabbs;
		std::vector<glm::uvec2> offsets;

		std::vector<AVulkanTextureImage> images;
		uint32_t numTextures = 0;

		for (const auto& model : models)
		{
			uint32_t indexOffset = indices.size();
			uint32_t vertexOffset = vertices.size();
			uint32_t materialOffset = materials.size();

			offsets.emplace_back(indexOffset, vertexOffset);

			vertices.insert(vertices.end(), model.vertexBuffer.begin(), model.vertexBuffer.end());
			indices.insert(indices.end(), model.indexBuffer.begin(), model.indexBuffer.end());
			materials.insert(materials.end(), model.materials.begin(), model.materials.end());

			for (int i = vertexOffset; i != vertices.size(); i++)
			{
				vertices[i].materialIndex += materialOffset;
			}

			numTextures += model.textures.size();
		}

		textureImages.reserve(numTextures);
		textureImageViewHandles.reserve(numTextures);
		textureSamplerHandles.reserve(numTextures);
		numTextures = 0;
		for (const auto& model : models)
		{
			for (int i = 0; i < model.textures.size(); i++)
			{
				const auto& image = model.images[model.textures[i].imageIndex];
				const auto& sampler = model.samplers[model.textures[i].samplerIndex];
				AddTextureImageFromGLTF(image, sampler);
			}

			numTextures += model.textures.size();
		}

		utility::BufferUtility::CreateBuffer(commandPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR 
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			vertices, vertexBuffer, vertexBufferMemory);

		utility::BufferUtility::CreateBuffer(commandPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR 
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			indices, indexBuffer, indexBufferMemory);

		utility::BufferUtility::CreateBuffer(commandPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
			materials, materialBuffer, materialBufferMemory);

		utility::BufferUtility::CreateBuffer(commandPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			offsets, offsetsBuffer, offsetsBufferMemory);
	}

	AScene::~AScene()
	{
		textureSamplerHandles.clear();
		textureImageViewHandles.clear();
		textureImages.clear();
		aabbBuffer.reset();
		aabbBufferMemory.reset();
		offsetsBuffer.reset();
		offsetsBufferMemory.reset();
		materialBuffer.reset();
		materialBufferMemory.reset();
		indexBuffer.reset();
		indexBufferMemory.reset();
		vertexBuffer.reset();
		vertexBufferMemory.reset();
	}

	void AScene::AddTextureImageFromGLTF(const AglTFModel::Image& image, const AglTFModel::Sampler& sampler)
	{

	}
}