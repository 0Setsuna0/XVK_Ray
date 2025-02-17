#include "XVKImage.h"
#include "XVKDevice.h"
#include "XVKCommandPool.h"

namespace xvk
{
	XVKImage::XVKImage(const XVKDevice& device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
		:xvk_device(device), vk_extent(extent), vk_format(format), vk_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
	}

	XVKImage::~XVKImage()
	{
	}

	void XVKImage::TransitionImageLayout(XVKCommandPool commandPool, VkImageLayout newLayout)
	{

	}
}