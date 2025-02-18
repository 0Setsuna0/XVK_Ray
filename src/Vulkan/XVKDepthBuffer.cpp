#include "XVKDepthBuffer.h"
#include "XVKDevice.h"
#include "XVKSwapChain.h"
#include "XVKImageView.h"
#include "XVKDeviceMemory.h"
#include "XVKCommandPool.h"
#include "XVKImage.h"
namespace xvk
{
	XVKDepthBuffer::XVKDepthBuffer(XVKCommandPool& commandPool, const VkExtent2D extent)
		: vk_depthFormat(VK_FORMAT_D32_SFLOAT)
	{
		const auto& device = commandPool.GetDevice();
		
		vk_depthImage.reset(new XVKImage(device, extent, vk_depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
		vk_depthImageMemory.reset(new XVKDeviceMemory(vk_depthImage->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		vk_depthImageView.reset(new XVKImageView(device, vk_depthImage->Handle(), vk_depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT));

		vk_depthImage->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	XVKDepthBuffer::~XVKDepthBuffer()
	{
		vk_depthImageView.reset();
		vk_depthImage.reset();
		vk_depthImageMemory.reset();
	}
}