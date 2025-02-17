#pragma once

#include "XVKCommon.h"
#include <memory>

namespace xvk
{
	class XVKDevice;
	class XVKSwapChain;
	class XVKImageView;
	class XVKDeviceMemory;
	class XVKCommandPool;

	class XVKDepthBuffer
	{
	public:
		XVKDepthBuffer(XVKCommandPool& commandPool, const VkExtent2D extent);
		~XVKDepthBuffer();

		VkFormat GetDepthFormat() const { return vk_depthFormat; }
		const VkImageView& GetDepthImageView() const { return *vk_depthImageView; }

	private:
		std::unique_ptr<VkImage> vk_depthImage;
		std::unique_ptr<VkImageView> vk_depthImageView;
		std::unique_ptr<VkDeviceMemory> vk_depthImageMemory;
		const VkFormat vk_depthFormat;
	};
}