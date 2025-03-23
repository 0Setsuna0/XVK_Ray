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
	AScene::AScene(xvk::XVKCommandPool& commandPool, const std::vector<AglTFModel*>& models)
		:models(models)
	{
		std::vector<AVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<AMaterial> materials;
		std::vector<VkAabbPositionsKHR> aabbs;
		std::vector<glm::uvec2> offsets;

		std::vector<AVulkanTextureImage> images;
		uint32_t numTextures = 0;

		//defaultModelViewMatrix = glm::lookAt(glm::vec3(0, -3, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
		defaultModelViewMatrix = glm::lookAt(glm::vec3(3.5, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		for (const auto& model : models)
		{
			uint32_t indexOffset = indices.size();
			uint32_t vertexOffset = vertices.size();
			uint32_t materialOffset = materials.size();

			offsets.emplace_back(indexOffset, vertexOffset);

			vertices.insert(vertices.end(), model->vertexBuffer.begin(), model->vertexBuffer.end());
			indices.insert(indices.end(), model->indexBuffer.begin(), model->indexBuffer.end());
			materials.insert(materials.end(), model->materials.begin(), model->materials.end());

			for (int i = vertexOffset; i != vertices.size(); i++)
			{
				vertices[i].materialIndex += materialOffset;
			}

			numTextures += model->textures.size();
		}

		textures.reserve(numTextures);
		textureImages.reserve(numTextures);
		textureImageViewHandles.reserve(numTextures);
		textureSamplerHandles.reserve(numTextures);
		
		for (const auto& model : models)
		{
			for (int i = 0; i < model->textures.size(); i++)
			{
				const auto& image = model->images[model->textures[i].imageIndex];
				AddTextureImageFromGLTF(commandPool, image);
			}
		}

		if (textures.empty())
		{
			textures.emplace_back(std::make_unique<vkAsset::AVulkanTexture>(
				ASSET_DIR"texture/TestTexture0.png", xvk::SamplerConfig()));
			textureImages.emplace_back(std::make_unique<AVulkanTextureImage>(commandPool, *textures.back()));
			textureImageViewHandles.emplace_back(textureImages.back()->GetTextureImageView().Handle());
			textureSamplerHandles.emplace_back(textureImages.back()->GetTextureImageSampler().Handle());
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

	void AScene::AddTextureImageFromGLTF(xvk::XVKCommandPool& commandPool, const AglTFModel::Image& image)
	{
		//todo: add glTFModel sampler support
		textures.emplace_back(std::make_unique<AVulkanTexture>(image.bufferData, image.width, image.height, xvk::SamplerConfig()));
		textureImages.emplace_back(std::make_unique<AVulkanTextureImage>(commandPool, image.bufferData,
			xvk::SamplerConfig(), image.width, image.height));
		textureImageViewHandles.emplace_back(textureImages.back()->GetTextureImageView().Handle());
		textureSamplerHandles.emplace_back(textureImages.back()->GetTextureImageSampler().Handle());
	}
}