#include "XVKImage.h"
#include "XVKDevice.h"
#include "XVKCommandPool.h"
#include "XVKDeviceMemory.h"
#include "XVKTransientCommand.h"
#include "XVKBuffer.h"

namespace xvk
{
	XVKImage::XVKImage(const XVKDevice& device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
		:xvk_device(device), vk_extent(extent), vk_format(format), vk_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = extent.width;
		imageInfo.extent.height = extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk_imageLayout;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		VULKAN_RUNTIME_CHECK(vkCreateImage(device.Handle(), &imageInfo, nullptr, &vk_image), "create image");
	}

	XVKImage::~XVKImage()
	{
		if (vk_image != VK_NULL_HANDLE)
		{
			vkDestroyImage(xvk_device.Handle(), vk_image, nullptr);
			vk_image = VK_NULL_HANDLE;
		}
	}

	XVKDeviceMemory XVKImage::AllocateMemory(VkMemoryPropertyFlags properties) const
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(xvk_device.Handle(), vk_image, &memRequirements);
		XVKDeviceMemory memory(xvk_device, memRequirements.size, memRequirements.memoryTypeBits, 0, properties);
		VULKAN_RUNTIME_CHECK(vkBindImageMemory(xvk_device.Handle(), vk_image, memory.Handle(), 0),
			"bind image memory");
		return memory;
	}

	//remember XVKCommandPool& commandPool not XVKCommandPool commandPool
	void XVKImage::TransitionImageLayout(XVKCommandPool& commandPool, VkImageLayout newLayout)
	{
		XVKTransientCommands::Submit(commandPool, [&](VkCommandBuffer commandBuffer) {
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = vk_imageLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = vk_image;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				if (vk_format == VK_FORMAT_D32_SFLOAT_S8_UINT || vk_format == VK_FORMAT_D24_UNORM_S8_UINT)
				{
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (vk_imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (vk_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (vk_imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
			{
				RUNTIME_ERROR("unsupported layout transition");
			}

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		});

		vk_imageLayout = newLayout;
	}

	void XVKImage::CopyFromBuffer(XVKCommandPool& commandPool, const XVKBuffer& buffer)
	{
		XVKTransientCommands::Submit(commandPool, [&](VkCommandBuffer commandBuffer)
		{
				VkBufferImageCopy region = {};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;
				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { vk_extent.width, vk_extent.height, 1 };

				vkCmdCopyBufferToImage(commandBuffer, buffer.Handle(), vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &region);
		});
	}
}