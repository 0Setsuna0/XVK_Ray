#pragma once

#include "Vulkan/XVKSampler.h"
#include <memory>
#include <string>
#include "Utility/ResourceLoader.h"

namespace vkAsset
{
	class AVulkanTexture
	{
	public:
		AVulkanTexture(const std::string& filePath, const xvk::SamplerConfig& samplerConfig);
		~AVulkanTexture() {}

		const stbi_uc* GetPixelData() const { return pixel_data.get(); }
		VkDeviceSize GetTextureSize() const { return height * width * 4; }
		VkExtent2D GetExtent() const { return textureExtent; }
		const xvk::SamplerConfig& GetSamplerConfig() const { return xvk_samplerConfig; }
	private:
		xvk::SamplerConfig xvk_samplerConfig;
		int width;
		int height;
		int channels;
		VkExtent2D textureExtent;
		std::unique_ptr<stbi_uc, void (*) (void*)> pixel_data;
	};
}

