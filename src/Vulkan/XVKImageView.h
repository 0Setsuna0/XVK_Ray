#pragma once
#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	class XVKImageView
	{
	public:
		XVKImageView(const XVKDevice& device, const VkImage image, const VkFormat format,
			const VkImageAspectFlags aspectFlags);
		~XVKImageView();

		VkImageView Handle() const { return vk_imageView; }
		const XVKDevice& GetDevice() const { return xvk_device; }
		const VkImage& GetImage() const { return vk_image; }

	private:
		const XVKDevice& xvk_device;
		VkImageView vk_imageView;
		VkImage vk_image;
		VkFormat vk_format;
	};
}