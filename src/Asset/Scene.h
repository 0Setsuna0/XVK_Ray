#pragma once

#include "Vulkan/XVKCommon.h"
#include <memory>
#include "Model.h"

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
		AScene(xvk::XVKCommandPool& commandPool, const std::vector<AglTFModel*>& models);
		~AScene();

		const std::vector<AglTFModel*>& GetModels() const { return models; }
		
		const xvk::XVKBuffer& GetVertexBuffer() const { return *vertexBuffer; }
		const xvk::XVKBuffer& GetIndexBuffer() const { return *indexBuffer; }
		const xvk::XVKBuffer& GetMaterialBuffer() const { return *materialBuffer; }
		const xvk::XVKBuffer& GetAABBBuffer() const { return *aabbBuffer; }
		const xvk::XVKBuffer& GetOffsetsBuffer() const { return *offsetsBuffer; }
		const std::vector<VkImageView> GetTextureImageViews() const { return textureImageViewHandles; }
		const std::vector<VkSampler> GetTextureSamplers() const { return textureSamplerHandles; }
		void AddTextureImageFromGLTF(xvk::XVKCommandPool& commandPoolconst, const AglTFModel::Image& image);
	
		bool HasSkyBox() const { return hasSkyBox; }
		glm::mat4 GetDefaultModelViewMatrix() const { return defaultModelViewMatrix; }

	private:
		
		bool hasSkyBox = false;
		glm::mat4 defaultModelViewMatrix;

		const std::vector<AglTFModel*>& models;
		std::vector<std::unique_ptr<AVulkanTexture>> textures;

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