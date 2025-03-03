#include "Texture.h"
#include <iostream>

namespace vkAsset
{
	AVulkanTexture::AVulkanTexture(const std::string& filePath, const xvk::SamplerConfig& samplerConfig)
		:xvk_samplerConfig(samplerConfig), pixel_data(nullptr, stbi_image_free)
	{
		stbi_uc* temp_pixel_data = stbi_load(filePath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
		pixel_data.reset(temp_pixel_data);
		if (!pixel_data)
		{
			RUNTIME_ERROR("Fail to load texture image");
		}

		textureExtent.width = width;
		textureExtent.height = height;
	}

	AVulkanTexture::AVulkanTexture(unsigned char* data, int width, int height, const xvk::SamplerConfig& samplerConfig)
		:xvk_samplerConfig(samplerConfig), pixel_data(data, stbi_image_free)
	{
		textureExtent.width = width;
		textureExtent.height = height;
	}
}