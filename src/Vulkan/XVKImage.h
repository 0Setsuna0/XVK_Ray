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
	private:
		VkImage vk_image;
		const XVKDevice& xvk_device;
		const VkExtent2D vk_extent;
		const VkFormat vk_format;
		VkImageLayout vk_imageLayout;
	};
}