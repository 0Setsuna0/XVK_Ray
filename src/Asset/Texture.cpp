#include "Texture.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace vkAsset
{
	ATexture::ATexture(const std::string& filePath, const xvk::SamplerConfig& samplerConfig)
		:xvk_samplerConfig(samplerConfig)
	{
		pixel_data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!pixel_data)
		{
			RUNTIME_ERROR("Fail to load texture image");
		}

		textureExtent.width = width;
		textureExtent.height = height;
	}

	void ATexture::FreePixelData()
	{
		stbi_image_free(pixel_data);
	}
}