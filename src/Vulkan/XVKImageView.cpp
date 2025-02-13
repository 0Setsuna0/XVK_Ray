#include "XVKImageView.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKImageView::XVKImageView(const XVKDevice& device, const VkImage image, const VkFormat format,
		const VkImageAspectFlags aspectFlags)
		: xvk_device(device), vk_image(image)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(xvk_device.Handle(), &createInfo, nullptr, &vk_imageView) != VK_SUCCESS)
		{
			RUNTIME_ERROR("Failed to create image view!");
		}
	}
	
	XVKImageView::~XVKImageView()
	{
		if (vk_image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(xvk_device.Handle(), vk_imageView, nullptr);
			vk_image = VK_NULL_HANDLE;
		}
	}
}