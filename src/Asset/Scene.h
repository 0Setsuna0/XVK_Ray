#pragma once

#include "Vulkan/XVKCommon.h"
#include <memory>

namespace xvk
{
	class XVKBuffer;
	class XVKCommandPool;
	class XVKImage;
	class XVKDeviceMemory;
	class XVKImageView;
	class XVKSampler;
}

namespace vkAsset
{
	class AglTFModel;
	class AVulkanTexture;
	class AVulkanTextureImage;

	class AScene
	{
	public:

		AScene(xvk::XVKCommandPool& commandPool, std::vector<AglTFModel>&& models, std::vector<AVulkanTexture>&& textures);
		~AScene();

		const std::vector<AglTFModel>& GetModels() const { return models; }
		
		const xvk::XVKBuffer& GetVertexBuffer() const { return *vertexBuffer; }
		const xvk::XVKBuffer& GetIndexBuffer() const { return *indexBuffer; }
		const xvk::XVKBuffer& GetMaterialBuffer() const { return *materialBuffer; }
		const xvk::XVKBuffer& GetAABBBuffer() const { return *aabbBuffer; }
		const xvk::XVKBuffer& GetOffsetsBuffer() const { return *offsetsBuffer; }
		const std::vector<VkImageView> TextureImageViews() const { return textureImageViewHandles; }
		const std::vector<VkSampler> TextureSamplers() const { return textureSamplerHandles; }
	private:
		const std::vector<AglTFModel> models;
		const std::vector<AVulkanTexture> textures;

		std::unique_ptr<xvk::XVKBuffer> vertexBuffer;
		std::unique_ptr<xvk::XVKDeviceMemory> vertexBufferMemory;

		std::unique_ptr<xvk::XVKBuffer> indexBuffer;
		std::unique_ptr<xvk::XVKDeviceMemory> indexBufferMemory;

		std::unique_ptr<xvk::XVKBuffer> materialBuffer;
		std::unique_ptr<xvk::XVKDeviceMemory> materialBufferMemory;

		std::unique_ptr<xvk::XVKBuffer> offsetsBuffer;
		std::unique_ptr<xvk::XVKDeviceMemory> offsetsBufferMemory;

		std::unique_ptr<xvk::XVKBuffer> aabbBuffer;
		std::unique_ptr<xvk::XVKDeviceMemory> aabbBufferMemory;

		std::vector<std::unique_ptr<AVulkanTextureImage>> textureImages;
		std::vector<VkImageView> textureImageViewHandles;
		std::vector<VkSampler> textureSamplerHandles;
	};
}