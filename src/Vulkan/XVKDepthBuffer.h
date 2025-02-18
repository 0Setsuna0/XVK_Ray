#pragma once

#include "XVKCommon.h"
#include <memory>

namespace xvk
{
	class XVKDevice;
	class XVKImageView;
	class XVKDeviceMemory;
	class XVKCommandPool;
	class XVKImage;

	class XVKDepthBuffer
	{
	public:
		XVKDepthBuffer(XVKCommandPool& commandPool, const VkExtent2D extent);
		~XVKDepthBuffer();

		VkFormat GetDepthFormat() const { return vk_depthFormat; }
		const XVKImageView& GetDepthImageView() const { return *vk_depthImageView; }

	private:
		std::unique_ptr<XVKImage> vk_depthImage;
		std::unique_ptr<XVKImageView> vk_depthImageView;
		std::unique_ptr<XVKDeviceMemory> vk_depthImageMemory;
		const VkFormat vk_depthFormat;
	};
}