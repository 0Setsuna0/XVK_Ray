#pragma once

#include <memory>

namespace xvk
{
	class XVKCommandPool;
	class XVKDeviceMemory;
	class XVKImage;
	class XVKImageView;
	class XVKSampler;
	struct SamplerConfig;
}

namespace vkAsset
{
	class AVulkanTexture;

	class AVulkanTextureImage
	{
	public:

		AVulkanTextureImage(xvk::XVKCommandPool& commandPool, const AVulkanTexture& texture);
		AVulkanTextureImage(xvk::XVKCommandPool& commandPool, unsigned char* data, const xvk::SamplerConfig& samplerConfig,
			uint32_t width, uint32_t height);
		~AVulkanTextureImage();

		const xvk::XVKImageView& GetTextureImageView() const { return *imageView; }
		const xvk::XVKSampler& GetTextureImageSampler() const { return *imageSampler; }
		const xvk::XVKImage& GetTextureImage() const { return *image; }
	private:
		std::unique_ptr<xvk::XVKImage> image;
		std::unique_ptr<xvk::XVKDeviceMemory> imageMemory;
		std::unique_ptr<xvk::XVKImageView> imageView;
		std::unique_ptr<xvk::XVKSampler> imageSampler;
	};
}