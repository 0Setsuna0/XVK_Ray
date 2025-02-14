#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKInstance;
	class XVKDevice;

	class XVKRenderPass
	{
	public:
		XVKRenderPass(const XVKDevice& device,
			const VkFormat colorBufferFormat,
			const VkFormat depthBufferFormat,
			const VkAttachmentLoadOp colorBufferLoadOp,
			const VkAttachmentLoadOp depthBufferLoadOp);
		~XVKRenderPass();

		VkRenderPass Handle() const { return vk_renderPass; }
		VkFormat GetColorFormat() const { return vk_colorFormat; }
		VkFormat GetDepthFormat() const { return vk_depthFormat; }
		const XVKDevice& GetDevice() const { return xvk_device; }

	private:
		const XVKDevice& xvk_device;

		VkRenderPass vk_renderPass;
		VkFormat vk_colorFormat;
		VkFormat vk_depthFormat;
	};
}