#pragma once
#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;
	class XVKCommandPool;
	class XVKDeviceMemory;
	class XVKBuffer;

	class XVKImage
	{
	public:
		XVKImage(const XVKDevice& device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
		~XVKImage();

		VkImage Handle() const { return vk_image; };
		VkExtent2D GetExtent() const { return vk_extent; };
		VkFormat GetFormat() const { return vk_format; };
		VkImageLayout GetImageLayout() const { return vk_imageLayout; };
		const XVKDevice& GetDevice() const { return xvk_device; };

		XVKDeviceMemory AllocateMemory(VkMemoryPropertyFlags properties) const;
		//VkMemoryRequirements GetMemoryRequirements() const;

		void TransitionImageLayout(XVKCommandPool& commandPool, VkImageLayout newLayout);
		void CopyFromBuffer(XVKCommandPool& commandPool, const XVKBuffer& buffer);

		static void Insert(
			const VkCommandBuffer commandBuffer,
			const VkImage image,
			const VkImageSubresourceRange subresourceRange,
			const VkAccessFlags srcAccessMask,
			const VkAccessFlags dstAccessMask,
			const VkImageLayout oldLayout,
			const VkImageLayout newLayout)
		{
			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = srcAccessMask;
			barrier.dstAccessMask = dstAccessMask;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
				&barrier);
		}

	private:
		VkImage vk_image;
		const XVKDevice& xvk_device;
		const VkExtent2D vk_extent;
		const VkFormat vk_format;
		VkImageLayout vk_imageLayout;
	};
}