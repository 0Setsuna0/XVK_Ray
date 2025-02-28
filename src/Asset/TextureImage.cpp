#include "TextureImage.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Vulkan/XVKImage.h"
#include "Vulkan/XVKImageView.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKSampler.h"
#include "Texture.h"

namespace vkAsset
{
	AVulkanTextureImage::AVulkanTextureImage(xvk::XVKCommandPool& commandPool, const AVulkanTexture& texture)
	{
		const VkDeviceSize imageSize = texture.GetExtent().width * texture.GetExtent().height * 4;
		const auto& device = commandPool.GetDevice();

		auto stagingBuffer = std::make_unique<xvk::XVKBuffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		auto stagingMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		void* data = stagingMemory.MapMemory(0, imageSize);
		memcpy(data, texture.GetPixelData(), imageSize);
		stagingMemory.UnmapMemory();

		image.reset(new xvk::XVKImage(device, texture.GetExtent(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));
		imageMemory.reset(new xvk::XVKDeviceMemory(image->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView.reset(new xvk::XVKImageView(device, image->Handle(), image->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT));
		imageSampler.reset(new xvk::XVKSampler(device, texture.GetSamplerConfig()));

		image->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image->CopyFromBuffer(commandPool, *stagingBuffer);
		image->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		stagingBuffer.reset();
	}

	AVulkanTextureImage::~AVulkanTextureImage()
	{
		imageSampler.reset();
		imageView.reset();
		image.reset();
		imageMemory.reset();
	}
}