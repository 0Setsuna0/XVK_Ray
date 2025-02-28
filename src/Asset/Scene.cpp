#include "Scene.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKImage.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Vulkan/XVKImageView.h"
#include "Vulkan/XVKSampler.h"
#include "Model.h"
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
		}
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
}