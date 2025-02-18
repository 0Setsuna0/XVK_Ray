#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKInstance;
	class XVKDevice;
	class XVKDepthBuffer;
	class XVKSwapChain;

	class XVKRenderPass
	{
	public:
		XVKRenderPass(const XVKDevice& device,
			const VkAttachmentLoadOp colorBufferLoadOp,
			const VkAttachmentLoadOp depthBufferLoadOp,
			const XVKDepthBuffer& depthBuffer,
			const XVKSwapChain& swapChain);
		~XVKRenderPass();

		VkRenderPass Handle() const { return vk_renderPass; }
		VkFormat GetColorFormat() const { return vk_colorFormat; }
		VkFormat GetDepthFormat() const { return vk_depthFormat; }
		const XVKDevice& GetDevice() const { return xvk_device; }
		const XVKDepthBuffer& GetDepthBuffer() const { return xvk_depthBuffer; }
		const XVKSwapChain& GetSwapChain() const { return xvk_swapChain; }

	private:
		const XVKDevice& xvk_device;
		const XVKDepthBuffer& xvk_depthBuffer;
		const XVKSwapChain& xvk_swapChain;

		VkRenderPass vk_renderPass;
		VkFormat vk_colorFormat;
		VkFormat vk_depthFormat;
	};
}