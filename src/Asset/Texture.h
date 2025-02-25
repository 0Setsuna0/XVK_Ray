#pragma once

#include "Vulkan/XVKSampler.h"
#include <memory>
#include <string>
#include <stb_image.h>

namespace vkAsset
{
	class ATexture
	{
	public:
		ATexture(const std::string& filePath, const xvk::SamplerConfig& samplerConfig);
		~ATexture() {}

		void FreePixelData();

		const stbi_uc* GetPixelData() const { return pixel_data; }
		VkDeviceSize GetTextureSize() const { return height * width * 4; }
		VkExtent2D GetExtent() const { return textureExtent; }

	private:
		xvk::SamplerConfig xvk_samplerConfig;
		int width;
		int height;
		int channels;
		VkExtent2D textureExtent;
		stbi_uc* pixel_data;
	};
}

