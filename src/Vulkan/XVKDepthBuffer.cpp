#include "XVKDepthBuffer.h"
#include "XVKDevice.h"
#include "XVKSwapChain.h"
#include "XVKImageView.h"
#include "XVKDeviceMemory.h"
#include "XVKCommandPool.h"

namespace xvk
{
	XVKDepthBuffer::XVKDepthBuffer(XVKCommandPool& commandPool, const VkExtent2D extent)
		: vk_depthFormat(VK_FORMAT_D32_SFLOAT)
	{
		const auto& device = commandPool.GetDevice();
		
		vk_depthImage.reset(new VkImage);

	}

	XVKDepthBuffer::~XVKDepthBuffer()
	{
	}
}