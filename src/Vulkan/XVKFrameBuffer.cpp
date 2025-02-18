#include "XVKFrameBuffer.h"
#include "XVKRenderPass.h"
#include "XVKImageView.h"
#include "XVKDevice.h"
#include "XVKSwapChain.h"
#include "XVKDepthBuffer.h"

namespace xvk
{
	XVKFrameBuffer::XVKFrameBuffer(const XVKImageView& imageView, const XVKRenderPass& renderPass)
		: xvk_imageView(imageView), xvk_renderPass(renderPass)
	{
		std::vector<VkImageView> attachments = { xvk_imageView.Handle(), renderPass.GetDepthBuffer().GetDepthImageView().Handle()};
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass.Handle();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = renderPass.GetSwapChain().GetExtent().width;
		framebufferInfo.height = renderPass.GetSwapChain().GetExtent().height;
		framebufferInfo.layers = 1;

		VULKAN_RUNTIME_CHECK(vkCreateFramebuffer(xvk_imageView.GetDevice().Handle(), &framebufferInfo, nullptr, &vk_frameBuffer),
			"create framebuffer");
	}

	XVKFrameBuffer::XVKFrameBuffer(XVKFrameBuffer&& other) noexcept
		: xvk_imageView(other.xvk_imageView), xvk_renderPass(other.xvk_renderPass), vk_frameBuffer(other.vk_frameBuffer)
	{
		other.vk_frameBuffer = VK_NULL_HANDLE;
	}

	XVKFrameBuffer::~XVKFrameBuffer()
	{
		if (vk_frameBuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(xvk_imageView.GetDevice().Handle(), vk_frameBuffer, nullptr);
			vk_frameBuffer = VK_NULL_HANDLE;
		}
	}	
}